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

#define TRUE 1
#define FALSE 0

#define PACOTE_BUFFER_SIZE 512

// tamanho maximo do nome do arquivo semeado ou baixado
#define MAX_NOME_ARQUIVO 200

// constantes de tipo de aviso ao servidor
#define CLIENTE_BAIXAR_ONLINE 2
#define CLIENTE_SENVIAR_ONLINE 1

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

void calc_checksum(Pacote *pct) { //Calcula a soma de verificacao
    unsigned int i, sum = 0;
    for (i = 0; i < PACOTE_BUFFER_SIZE; i++) {
        if (pct->buffer[i] == '1') sum += 2*i;
        else sum += i;
    }

    pct->checksum = sum;
}

void enviar(int sockfd, struct sockaddr_in *servaddr) {

    char nome_arquivo[MAX_NOME_ARQUIVO];
    bzero(nome_arquivo, MAX_NOME_ARQUIVO);
    int aviso = CLIENTE_SENVIAR_ONLINE;
    struct sockaddr_in baixadoraddr;
    bzero(&baixadoraddr, sizeof(baixadoraddr));
    socklen_t l = sizeof(struct sockaddr);
    socklen_t l2 = sizeof(baixadoraddr);
    FILE * fp;
    int nome_valido = FALSE;
    // avisa servidor dizendo que esta online esperando um cliente enviador
    if(sendto(sockfd, &aviso, sizeof(aviso), 0, (struct sockaddr * ) servaddr, sizeof(struct sockaddr)) < 0) {
        error("erro ao enviar aviso\n");
    }

    printf("esperando servidor encontrar cliente que ira baixar...\n");
    // espera servidor retornar informações do cliente enviador
    if(recvfrom(sockfd, &baixadoraddr, l2, 0, (struct sockaddr * ) servaddr, &l) < 0) {
       error("erro ao receber pacote\n");
    }

    printf("servidor encontrou cliente baixador ");
    printf("ip: %s\n", inet_ntoa(baixadoraddr.sin_addr));
    printf("esperando cliente_baixar digitar nome do arquivo\n");

    // recebe nome do arquivo e verifica se é valido, se nao for pede nome de novo
    while(nome_valido == FALSE) {
        // recebe nome do arquivo desejado
        if(recvfrom(sockfd, nome_arquivo, MAX_NOME_ARQUIVO, 0, (struct sockaddr * ) &baixadoraddr, &l2) < 0) {
            error("erro ao receber nome do arquivo\n");
        }

        fp = fopen(nome_arquivo, "rb");
        if (fp == NULL) {
            printf("arquivo procurado pelo cliente_baixar nao existe\n");
            nome_valido = FALSE;
        } else {
            nome_valido = TRUE;
        }
        if (sendto(sockfd, &nome_valido, sizeof(nome_valido), 0, (struct sockaddr * ) &baixadoraddr, sizeof(baixadoraddr)) < 0) {
            error("erro ao enviar nome_valido\n");
        }

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

        calc_checksum(&pacotes[seq]); //Calcula o checksum do pacote atual

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
    printf("\narquivo %s enviado com sucesso!\n", nome_arquivo);
    fclose(fp);

}

int main() {
    int sockfd; // socket do client
    struct sockaddr_in servaddr; // endereco do servidor

    // inicializa socket
    initsocket(&sockfd, &servaddr);
    enviar(sockfd, &servaddr);
    close(sockfd);

    return (0);
}
