#include <sys/socket.h>
#include <netinet/in.h>
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

// constantes de tipo de aviso ao servidor
#define CLIENTE_BAIXAR_ONLINE 2
#define CLIENTE_SEMEAR_ONLINE 1


typedef struct cliente {
    struct sockaddr_in addr;
    int online;
} Cliente;

void error(char *msg) {
    printf("%s\n",msg);
    exit(1);
}

void initServer(int *sockfd, struct sockaddr_in *servaddr) {

    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sockfd == -1) {
        printf(" socket not created in server\n");
        exit(0);
    } 
    bzero((char *)servaddr, sizeof(servaddr));
    servaddr->sin_family = AF_INET;
    servaddr->sin_addr.s_addr = INADDR_ANY;
    servaddr->sin_port = htons(7802);

}

int main() {
    int sockfd; 
    socklen_t l = sizeof(struct sockaddr_in);
    struct sockaddr_in servaddr;
    struct sockaddr_in clienteaddr;    
    bzero(&(clienteaddr), sizeof(clienteaddr));

    Cliente cliente_baixar;
    bzero(&(cliente_baixar.addr), sizeof(cliente_baixar.addr));
    cliente_baixar.online = FALSE;

    Cliente cliente_semear;
    bzero(&(cliente_semear.addr), sizeof(cliente_semear.addr));
    cliente_semear.online = FALSE;

    int aviso = 0; // aviso que servidor recebe do cliente para saber qual esta online

    // cria socket do servidor
    initServer(&sockfd, &servaddr);
    if (bind(sockfd, (struct sockaddr * ) &servaddr, sizeof(servaddr)) != 0)
        printf("erro no bind\n");
    printf("servidor inicializado\n");
    printf("esperando clientes...\n");

    while(cliente_baixar.online == FALSE  || cliente_semear.online == FALSE) {
        // recebe aviso do cliente
        if(recvfrom(sockfd, &aviso, sizeof(aviso), 0, (struct sockaddr * ) &clienteaddr, &l) < 0) {
            error("erro ao receber aviso do cliente");
        }

        // verifica qual cliente entrou
        if(aviso == CLIENTE_BAIXAR_ONLINE && cliente_baixar.online == FALSE) {
            cliente_baixar.addr = clienteaddr;
            cliente_baixar.online = TRUE;
            bzero(&(clienteaddr), sizeof(clienteaddr));
            printf("cliente baixar entrou\n");
        } 
        else if(aviso == CLIENTE_SEMEAR_ONLINE && cliente_semear.online == FALSE) {
            cliente_semear.addr = clienteaddr;
            cliente_semear.online = TRUE;
            bzero(&(clienteaddr), sizeof(clienteaddr));
            printf("cliente semear entrou\n");
        } 
        else if(aviso == CLIENTE_BAIXAR_ONLINE) {
            printf("já existe um cliente baixar online\n");
        } 
        else if(aviso == CLIENTE_SEMEAR_ONLINE) {
            printf("já existe um cliente semear online\n");
        }
    }

    // envia informacoes do cliente_semear para cliente_baixar
    if(sendto(sockfd, &(cliente_semear.addr), sizeof(cliente_semear.addr), 0, (struct sockaddr * ) &(cliente_baixar.addr), sizeof(cliente_baixar.addr)) < 0) {
        error("erro ao enviar info cliente_baixar\n");
    }
    printf("enviou endereco do semear para baixar\n");

    // envia informacoes do cliente_baixar para cliente_semear
    if(sendto(sockfd, &(cliente_baixar.addr), sizeof(cliente_baixar.addr), 0, (struct sockaddr * ) &(cliente_semear.addr), sizeof(cliente_semear.addr)) < 0) {
        error("erro ao enviar info cliente_semear\n");
    }
    printf("enviou endereco do baixar para semear\n");

    close(sockfd);
    return (0);
}
