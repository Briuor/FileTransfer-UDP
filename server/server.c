#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>


int main() {

    char buff[2000];
    char file_buffer[2000];
    int sd, connfd, len;

    struct sockaddr_in servaddr, cliaddr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sd == -1) {
        printf(" socket not created in server\n");
        exit(0);
    } else {
        printf("socket created in  server\n");
    }

    bzero( & servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(7802);

    if (bind(sd, (struct sockaddr * ) & servaddr, sizeof(servaddr)) != 0)
        printf("Not binded\n");
    else
        printf("Binded\n");

    len = sizeof(cliaddr);

    recvfrom(sd, buff, 1024, 0,
        (struct sockaddr * ) & cliaddr, & len);

    printf("%s\n", buff);
    /* */
    FILE * fp;
    fp = fopen(buff, "r");
    if (fp == NULL) {
        printf("file does not exist\n");
    }

    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fread(file_buffer, file_size, 1, fp) <= 0) {
        printf("unable to copy file into buffer\n");
        exit(1);
    }

    printf("%d", file_size);
    if (sendto(sd, file_buffer, strlen(file_buffer), 0, (struct sockaddr * ) & cliaddr, len) < 0) {
        printf("error in sending the file\n");
        exit(1);
    }
    bzero(file_buffer, sizeof(file_buffer));

    /* */
    close(sd);
    fclose(fp);
    return (0);
}