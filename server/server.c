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
#define CLIENTE_enviar_ONLINE 1


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

    Cliente cliente_enviar;
    bzero(&(cliente_enviar.addr), sizeof(cliente_enviar.addr));
    cliente_enviar.online = FALSE;

    int aviso = 0; // aviso que servidor recebe do cliente para saber qual esta online

    // cria socket do servidor
    initServer(&sockfd, &servaddr);
    if (bind(sockfd, (struct sockaddr * ) &servaddr, sizeof(servaddr)) != 0)
        printf("erro no bind\n");
    printf("servidor inicializado\n");
    printf("esperando clientes baixar e enviar entrarem...\n");

    while(cliente_baixar.online == FALSE  || cliente_enviar.online == FALSE) {
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
        else if(aviso == CLIENTE_enviar_ONLINE && cliente_enviar.online == FALSE) {
            cliente_enviar.addr = clienteaddr;
            cliente_enviar.online = TRUE;
            bzero(&(clienteaddr), sizeof(clienteaddr));
            printf("cliente enviar entrou\n");
        } 
    }

    // envia informacoes do cliente_enviar para cliente_baixar
    if(sendto(sockfd, &(cliente_enviar.addr), sizeof(cliente_enviar.addr), 0, (struct sockaddr * ) &(cliente_baixar.addr), sizeof(cliente_baixar.addr)) < 0) {
        error("erro ao enviar info cliente_baixar\n");
    }
    printf("enviou endereco do cliente_enviar para cliente_baixar\n");

    // envia informacoes do cliente_baixar para cliente_enviar
    if(sendto(sockfd, &(cliente_baixar.addr), sizeof(cliente_baixar.addr), 0, (struct sockaddr * ) &(cliente_enviar.addr), sizeof(cliente_enviar.addr)) < 0) {
        error("erro ao enviar info cliente_enviar\n");
    }
    printf("enviou endereco do cliente_baixar para cliente_enviar\n");

    close(sockfd);
    return (0);
}
