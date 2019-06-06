#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include<netinet/in.h>
#include<sys/types.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>

#define PACOTE_BUFFER_SIZE 100
#define TRUE 1
#define FALSE 0

typedef struct pacote {
    char buffer[PACOTE_BUFFER_SIZE];
    int ack;
    int seq;
} Pacote;

void error(char *msg) {
    printf("%s\n",msg);
    exit(1);
}

int main() {
    char buff[2000];
    int sockfd, connfd, len;
    FILE * fp;
    struct sockaddr_in servaddr, cliaddr;
    char new_file[] = "copied";

    // cria socket do cliente
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd == -1) {
        error("erro ao criar socket");
    } 

    bzero( & servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY; // qualquer endereço
    servaddr.sin_port = htons(7802); // porta

    // digita nome do arquivo
    printf("Nome do arquivo: \n");
    scanf(" %s", buff);

    // envia nome do arquivo para servidor
    sendto(sockfd, buff, strlen(buff), 0,
        (struct sockaddr * ) & servaddr, sizeof(struct sockaddr));
    int l = sizeof(struct sockaddr);

    strcat(new_file, buff);
    fp = fopen(new_file, "wb");
    // ---------------------------pacote-------------
    Pacote pacote;
    bzero(pacote.buffer, PACOTE_BUFFER_SIZE);
    int seq = 0;
    while(1) {
        // recebe pacote
        if (recvfrom(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) & servaddr, & l) < 0) {
            error("erro ao receber pacote\n");
        }
        printf("seq %d, pacote.seq %d, pacote.ack %d\n", seq, pacote.seq, pacote.ack);
        // se checksum deu errado pede de novo
        // while(!checksum()) {
            // if (recvfrom(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) & servaddr, & l) < 0) {
            //     printf("error in recieving the file\n");
            //     exit(1);
            // }
        // }
        // preenche ack do pacote recebido
        pacote.ack = 1;
        seq++;
        // envia pacote com ack = 1 para o servidor
        if (sendto(sockfd, &pacote, sizeof(Pacote), 0, (struct sockaddr * ) & servaddr, sizeof(struct sockaddr)) < 0) {
            error("erro ao enviar pacote\n");
        }

        // escreve arquivo
        if (fwrite(pacote.buffer, 1, PACOTE_BUFFER_SIZE, fp) < 0) {
            error("erro ao escrever arquivo\n");
        }

        // reseta pacote
        bzero(pacote.buffer, PACOTE_BUFFER_SIZE);
        pacote.ack = 0;
    }

    close(sockfd);
    fclose(fp);
    return (0);
}