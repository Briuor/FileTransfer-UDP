//--------------------------------------------------------------------
//- Instrucoes de Execucao em: https://github.com/Briuor/FileTransfer-UDP
//-                                                                  -  
//- Transferidor de arquivos entre clientes com o auxílio de um      -
//- um servidor que intermedia a conexão de dois clientes,           -
//- baseado na arquitetura P2P. O protocolo utilizado foi o UDP,     -
//- portanto, os artifícios para  garantia de integridade e segurança-
//- foram implementados na camada de aplicação.                      -
//-                                                                  -
//- Alunos:                                                          -
//-    Bruno Fernando Lopes - 2017014669                             -
//-    Érick de Oliveira Teixeira - 2017001437                       -
//--------------------------------------------------------------------
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0

#define PACOTE_BUFFER_SIZE 512

// tamanho maximo do nome do arquivo enviado ou baixado
#define MAX_NOME_ARQUIVO 200

// constantes de escolha do usuario
#define ENVIAR 1
#define BAIXAR 2
#define SAIR 3
#define OPCAO_INVALIDA 0

// constantes de tipo de aviso ao servidor
#define CLIENTE_BAIXAR_ONLINE 2
#define CLIENTE_ENVIAR_ONLINE 1

typedef struct pacote {
    char buffer[PACOTE_BUFFER_SIZE]; // buffer de bytes
    int ack; // flag de reconhecimento
    int seq; // numero de sequencia
    int ultimo; // flag para verificar se eh o ultimo pacote
    int checksum; //Soma de Verificação
} Pacote;

// dispara mensagem de erro e finaliza execucao
void error(char *msg) {
    printf("%s\n",msg);
    exit(1);
}

// inicializa socket e servidor
void initsocket(int *sockfd, struct sockaddr_in *servaddr) {
    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (*sockfd == -1) {
        error("erro ao criar socket");
    } 

    bzero( (char *)servaddr, sizeof(servaddr));

    servaddr->sin_family = AF_INET;
    servaddr->sin_addr.s_addr = INADDR_ANY; // qualquer endereço
    servaddr->sin_port = htons(7802); // porta

    printf("\nInicializacao bem sucedida!\n\n");
}

int verifica_checksum(Pacote *pct) { //Verifica o checksum
    unsigned int i, sum = 0;
    for (i = 0; i < PACOTE_BUFFER_SIZE; i++) {
        if (pct->buffer[i] == '1') sum += 2*i;
        else sum += i;
    }
    
    if (sum == pct->checksum) return TRUE;
    else return FALSE;
}

void baixar(int sockfd, struct sockaddr_in *servaddr) {

    char nome_arquivo[MAX_NOME_ARQUIVO]; // nome do arquivo requisitado
 
    // inicializa pacote (esse pacote no cliente que baixa o arquivo eh necessario para 
    // preencher informacoes(ack) do pacote recebido e enviar de volta para o servidor)
    Pacote pacote;
    pacote.ack = FALSE;
    pacote.ultimo = FALSE;
    bzero(pacote.buffer, PACOTE_BUFFER_SIZE);

    int seq = 0; // verificador de numero de sequencia
    socklen_t l = sizeof(struct sockaddr);
    FILE *f;
    int aviso = CLIENTE_BAIXAR_ONLINE; // aviso que sera enviado para servidor dizendo que clinte que vai baixar esta online
    struct sockaddr_in enviadoraddr; // informacoes do cliente enviador
    int checksum_ok = 0; //Flag para verificação do checksum
    int nome_valido = FALSE;

    // avisa servidor dizendo que esta online esperando um cliente enviador
    if(sendto(sockfd, &aviso, sizeof(aviso), 0, (struct sockaddr * ) servaddr, sizeof(struct sockaddr)) < 0) {
        error("erro ao enviar aviso\n");
    }

    printf("esperando servidor encontrar enviador...\n");
    // espera servidor retornar informações do cliente enviador
    if(recvfrom(sockfd, &enviadoraddr, l, 0, (struct sockaddr * ) servaddr, &l) < 0) {
       error("erro ao receber pacote\n");
    }

    printf("servidor encontrou cliente enviador ");
    printf("ip: %s\n", inet_ntoa(enviadoraddr.sin_addr));

    // se arquivo invalido digita nome de novo
    while(nome_valido == FALSE) {
        // digita nome do arquivo a ser baixado
        printf("Digite o nome do arquivo desejado: ");
        scanf(" %s", nome_arquivo);

        // envia nome do arquivo desejado para o cliente enviador
        if(sendto(sockfd, nome_arquivo, MAX_NOME_ARQUIVO, 0, (struct sockaddr * ) &enviadoraddr, sizeof(enviadoraddr)) < 0) {
            error("erro ao enviar nome_arquivo\n");
        }
        // recebe confirmacao de nome do arquivo valido
        if(recvfrom(sockfd, &nome_valido, sizeof(nome_valido), 0, (struct sockaddr * ) &enviadoraddr, &l) < 0) {
           error("erro ao receber nome_valido\n");
        }
        if(nome_valido == FALSE) 
            printf("arquivo de nome %s inexistente\n", nome_arquivo);

    }

    // abre arquivo a ser escrito com os dados dos pacotes recebidos
    char new[] = "copied";
    strcat(new, nome_arquivo);
    f = fopen(new, "wb");

    // inicia recebimento de pacotes, ate receber o ultimo pacote
    while(pacote.ultimo == FALSE) {
        // recebe pacote
        if(recvfrom(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) &enviadoraddr, &l) < 0) {
            error("erro ao receber pacote\n");
        }
        
        checksum_ok = verifica_checksum(&pacote); //Verifica a soma de verificação

        printf("seq %d, ack %d, checksum = %d\n", seq, pacote.ack, pacote.checksum); //Imprime informações referentes ao pacote recebido
        
        // se checksum deu errado, pede novamente o mesmo pacote
        while(checksum_ok != TRUE) {
            if (recvfrom(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) & enviadoraddr, & l) < 0) {
                printf("error in recieving the file\n");
                exit(1);
            }
        }

        // verifica numero de sequencia para ver se pacotes estao chegando ordenados
        if(seq == pacote.seq) {
            // preenche pacote com o ack do pacote recebido e incrementa numero de sequencia
            pacote.ack = 1;
            seq++;

            // envia pacote com ack = 1 para o servidor
            if (sendto(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) &enviadoraddr, sizeof(enviadoraddr)) < 0) {
                error("erro ao enviar pacote\n");
            }

            // escreve arquivo
            if (fwrite(pacote.buffer, 1, PACOTE_BUFFER_SIZE, f) < 0) {
                error("erro ao escrever arquivo\n");
            }

            // reseta pacote para ser preenchido com informações do proximo
            bzero(pacote.buffer, PACOTE_BUFFER_SIZE);
            pacote.ack = 0;
        }
    }
    printf("arquivo %s baixado com sucesso!\n", new);
    fclose(f);
}

int main() {
    int sockfd; // socket do client
    struct sockaddr_in servaddr; // endereco do servidor

    initsocket(&sockfd, &servaddr);
    baixar(sockfd, &servaddr);
    close(sockfd);

    return (0);
}
