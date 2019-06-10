#include <stdio.h>
#include <errno.h>
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

// tamanho maximo do nome do arquivo semeado ou baixado
#define MAX_NOME_ARQUIVO 200

// constantes de escolha do usuario
#define SEMEAR 1
#define BAIXAR 2
#define SAIR 3
#define OPCAO_INVALIDA 0

// constantes de tipo de aviso ao servidor
#define CLIENTE_BAIXAR_ONLINE 2
#define CLIENTE_SEMEAR_ONLINE 1

typedef struct pacote {
    char buffer[PACOTE_BUFFER_SIZE]; // buffer de bytes
    int ack; // flag de reconhecimento
    int seq; // numero de sequencia
    int ultimo; // flag para verificar se eh o ultimo pacote
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

// mostra o menu e atribui o valor da opcao escolhida ao parametro recebido
void menu(int *opcao_escolhida) {
    printf("--------OPCOES--------\n");
    printf("1) Semear arquivo\n");
    printf("2) Baixar arquivo\n");
    printf("3) Sair\n");
    printf("---------------------\n");
    printf("Digite o numero da opcao desejada: \n");    
    scanf(" %d", opcao_escolhida);
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
    struct sockaddr_in semeadoraddr; // informacoes do cliente semeador

    // avisa servidor dizendo que esta online esperando um cliente semeador
    if(sendto(sockfd, &aviso, sizeof(aviso), 0, (struct sockaddr * ) servaddr, sizeof(struct sockaddr)) < 0) {
        error("erro ao enviar aviso\n");
    }

    printf("esperando servidor encontrar semeador...\n");
    // espera servidor retornar informações do cliente semeador
    if(recvfrom(sockfd, &semeadoraddr, l, 0, (struct sockaddr * ) servaddr, &l) < 0) {
       error("erro ao receber pacote\n");
    }

    printf("servidor encontrou cliente semeador");
    printf("ip: %s\n", inet_ntoa(semeadoraddr.sin_addr));
    // digita nome do arquivo a ser baixado
    printf("Digite o nome do arquivo desejado: ");
    scanf(" %s", nome_arquivo);

    // envia nome do arquivo desejado para o cliente semeador
    if(sendto(sockfd, nome_arquivo, MAX_NOME_ARQUIVO, 0, (struct sockaddr * ) &semeadoraddr, sizeof(semeadoraddr)) < 0) {
        error("erro ao enviar nome_arquivo\n");
    }

    // abre arquivo a ser escrito com os dados dos pacotes recebidos
    char new[] = "copied";
    strcat(new, nome_arquivo);
    f = fopen(new, "wb");

    // inicia recebimento de pacotes, ate receber o ultimo pacote
    while(pacote.ultimo == FALSE) {
        // recebe pacote
        if(recvfrom(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) &semeadoraddr, &l) < 0) {
            error("erro ao receber pacote\n");
        }
        printf("seq %d, pacote.seq %d, pacote.ack %d\n", seq, pacote.seq, pacote.ack);
        // se checksum deu errado pede de novo
        // while(!checksum()) {
            // if (recvfrom(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) & semeadoraddr, & l) < 0) {
            //     printf("error in recieving the file\n");
            //     exit(1);
            // }
        // }

        // verifica numero de sequencia para ver se pacotes estao chegando ordenados
        if(seq == pacote.seq) {
            // preenche pacote com o ack do pacote recebido e incrementa numero de sequencia
            pacote.ack = 1;
            seq++;

            // envia pacote com ack = 1 para o servidor
            if (sendto(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) &semeadoraddr, sizeof(semeadoraddr)) < 0) {
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
    fclose(f);
}

void semear(int sockfd, struct sockaddr_in *servaddr) {

    char nome_arquivo[MAX_NOME_ARQUIVO];
    bzero(nome_arquivo, MAX_NOME_ARQUIVO);
    int aviso = CLIENTE_SEMEAR_ONLINE;
    struct sockaddr_in baixadoraddr;
    bzero(&baixadoraddr, sizeof(baixadoraddr));
    socklen_t l = sizeof(struct sockaddr);
    socklen_t l2 = sizeof(baixadoraddr);
    // avisa servidor dizendo que esta online esperando um cliente semeador
    if(sendto(sockfd, &aviso, sizeof(aviso), 0, (struct sockaddr * ) servaddr, sizeof(struct sockaddr)) < 0) {
        error("erro ao enviar aviso\n");
    }

    printf("esperando servidor encontrar cliente que ira baixar...\n");
    // espera servidor retornar informações do cliente semeador
    if(recvfrom(sockfd, &baixadoraddr, l2, 0, (struct sockaddr * ) servaddr, &l) < 0) {
       error("erro ao receber pacote\n");
    }

    printf("servidor encontrou cliente baixador ");
    printf("ip: %s\n", inet_ntoa(baixadoraddr.sin_addr));

    // recebe nome do arquivo desejado
    if(recvfrom(sockfd, nome_arquivo, MAX_NOME_ARQUIVO, 0, (struct sockaddr * ) &baixadoraddr, &l2) < 0) {
    printf("nome %s\n", nome_arquivo); 
        error("erro ao receber nome do arquivo\n");
    }

    FILE * fp;
    fp = fopen(nome_arquivo, "rb");
    if (fp == NULL) {
        printf("arquivo nao existe\n");
    }

    // pega tamanho do arquivo
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    int fr; // leitor de arquivo
    int numPacotes = (file_size / PACOTE_BUFFER_SIZE);
    printf("numPacotes: %d\n", numPacotes);
    int i;
    int seq = 0;
    Pacote pacotes[numPacotes]; // array de pacotes a ser enviado
    for(i = 0; i < numPacotes; i++) {
        bzero(pacotes[i].buffer, PACOTE_BUFFER_SIZE);
        pacotes[i].ack = FALSE;
        pacotes[i].seq = i;
        if(i == numPacotes - 1) pacotes[i].ultimo = TRUE;
        else pacotes[i].ultimo = FALSE;

    }
    printf("init pacote\n");
    Pacote pacoter; // pacote recebido do cliente que baixa com ack preenchido ou nao

    // inicia envio de pacotes
    for(seq = 0; seq < numPacotes ; seq++) {
        // lê arquivo
        if ((fr = fread(pacotes[seq].buffer, PACOTE_BUFFER_SIZE, 1, fp)) < 0) {
            error("erro ao pegar bytes do arquivo\n");
        }

        // envia pacote
        if (sendto(sockfd, &pacotes[seq], sizeof(Pacote), 0, (struct sockaddr * ) &baixadoraddr, sizeof(baixadoraddr)) < 0) {
            error("erro ao enviar pacote\n");
        }

        // verifica se cliente recebeu através do ack preenchido pelo cliente
        while(1) {
            // recebe o mesmo pacote que enviou pro cliente só que com ack = true preenchido pelo cliente
            if(recvfrom(sockfd, &pacoter, sizeof(Pacote), 0, (struct sockaddr * ) & baixadoraddr, &l) < 0){
                error("erro ao receber pacote\n");
            }
            pacotes[seq].ack = pacoter.ack;
            printf("seq: %d, pacote.seq: %d pacote.ack: %d\n", seq, pacotes[seq].seq, pacotes[seq].ack);
            // se ack == false cliente nao recebeu, entao manda de novo
            if(pacotes[seq].ack == FALSE) {
                if (sendto(sockfd, &pacotes[seq], sizeof(Pacote), 0, (struct sockaddr * ) &baixadoraddr, sizeof(baixadoraddr)) < 0) {
                    error("erro ao enviar pacote\n");
                }
            }
            // se ack == true sai do loop e envia proximo pacote
            else {
                break;
            }
        }
        printf("cliente recebeu numSeq: %d\n", pacotes[seq].seq);
    }
    fclose(fp);

}

int main() {
    int sockfd; // socket do client
    struct sockaddr_in servaddr; // endereco do servidor
    int opcao_escolhida = OPCAO_INVALIDA;

    // inicializa socket
    initsocket(&sockfd, &servaddr);

    // enquanto usuario digitar opcao invalida mostra menu novamente
    while(opcao_escolhida == OPCAO_INVALIDA) {
        // mostra menu de opcoes e obtem opcao escolhida pelo usuario
        menu(&opcao_escolhida);
        // lida com opcao escolhida (1-SEMEAR, 2-BAIXAR, 3-SAIR)
        switch(opcao_escolhida) {
            case SEMEAR:
                semear(sockfd, &servaddr);
                break;
            case BAIXAR:
                baixar(sockfd, &servaddr);
                break;
            case SAIR:
                close(sockfd);
                return 0;
            default:
                printf("\n\nOpcao invalida, digite 1-para semear, 2-para baixar ou 3-para sair\n\n");
                opcao_escolhida = OPCAO_INVALIDA;
        }   
    }

    close(sockfd);
    return (0);
}
