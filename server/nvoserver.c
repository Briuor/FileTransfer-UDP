#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PACOTE_BUFFER_SIZE 100
#define TRUE 1
#define FALSE 0

typedef Cliente {
    struct sockaddr clienteaddr;
    int tipo;
} cliente;

int main() {

    char buff[2000];
    int sockfd; 
    int l = sizeof(struct sockaddr_in);
    struct sockaddr_in servaddr, cliaddr;

    // cria socket do servidor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf(" socket not created in server\n");
        exit(0);
    } 
    bzero( & servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(7802);
    if (bind(sockfd, (struct sockaddr * ) & servaddr, sizeof(servaddr)) != 0)
        printf("erro no bind\n");

    // recebe nome do arquivo desejado
    recvfrom(sockfd, buff, 1024, 0,
        (struct sockaddr * ) & cliaddr, & l);


    close(sockfd);
    return (0);
}
