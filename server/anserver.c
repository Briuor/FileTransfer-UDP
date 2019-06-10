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
    int sd, connfd, len;

    struct sockaddr_in servaddr, cliaddr;
    len = sizeof(cliaddr);

    // cria socket do servidor
    sd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sd == -1) {
        printf(" socket not created in server\n");
        exit(0);
    } 

    bzero( & servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(7802);

    if (bind(sd, (struct sockaddr * ) & servaddr, sizeof(servaddr)) != 0)
        printf("erro no bind\n");

    // recebe nome do arquivo desejado
    recvfrom(sd, buff, 1024, 0,
        (struct sockaddr * ) & cliaddr, & len);

    printf("Nome do arquivo: %s\n", buff);

    FILE * fp;
    fp = fopen(buff, "rb");
    if (fp == NULL) {
        printf("arquivo nao existe\n");
    }

    // pega tamanho do arquivo
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    int fr; // leitor de arquivo
    // ---------------------------pacote-------------
    int numPacotes = (file_size / PACOTE_BUFFER_SIZE);
    int i, j;
    int seq = 0;
    Pacote pacotes[numPacotes]; // array de pacotes a ser enviado
    for(i = 0; i < numPacotes; i++) {
        bzero(pacotes[i].buffer, PACOTE_BUFFER_SIZE);
        pacotes[i].ack = 0;
        pacotes[i].seq = i;
    }
    Pacote pacoter; // pacote recebido do cliente
    for(seq = 0; seq < numPacotes ; seq++) {
        // lê arquivo
        if ((fr = fread(pacotes[seq].buffer, PACOTE_BUFFER_SIZE, 1, fp)) < 0) {
            error("erro ao pegar bytes do arquivo\n");
        }

        // envia pacote
        if (sendto(sd, &pacotes[seq], sizeof(Pacote), 0, (struct sockaddr * ) &cliaddr, len) < 0) {
            error("erro ao enviar pacote\n");
        }

        // verifica se cliente recebeu
        while(1) {
            if(recvfrom(sd, &pacoter, sizeof(Pacote), 0, (struct sockaddr * ) & cliaddr, & len) < 0){
                error("erro ao receber pacote\n");
            }
            pacotes[seq].ack = pacoter.ack;
            printf("seq: %d, pacote.seq: %d pacote.ack: %d\n", seq, pacotes[seq].seq, pacotes[seq].ack);
            // se ack == false manda de novo
            if(pacotes[seq].ack == FALSE) {
                if (sendto(sd, &pacotes[seq], sizeof(Pacote), 0, (struct sockaddr * ) &cliaddr, len) < 0) {
                    error("erro ao enviar pacote\n");
                }
            }
            // se ack true sai do loop
            else {
                break;
            }
        }
        printf("cliente recebeu numSeq: %d\n", pacotes[seq].seq);
    }
    if(feof(fp))
        printf("Fim do arquivo!\n");
    printf("Tamanho total: %d\n",file_size );
    printf("ftell: %ld\n",ftell(fp) );
    printf("utlimo pacote: %d\n",seq );
    printf("numPacotes: %d\n", numPacotes);
    printf("totalBytes: %d\n", numPacotes * PACOTE_BUFFER_SIZE);
    printf("Tamanho - totalBytes: %d\n", file_size - (numPacotes * PACOTE_BUFFER_SIZE));

    close(sd);
    fclose(fp);
    return (0);
}
