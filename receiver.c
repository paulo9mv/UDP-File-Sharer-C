#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include <arpa/inet.h>
#include<sys/types.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */
#include<pthread.h>
#include <time.h>
#include<errno.h>

typedef int bool;
#define true 1
#define false 0
#define PORT 67
#define BUFSIZE 101
#define SOCKET_ERROR -1
#define SOCKET_READ_TIMEOUT_SEC 2

void kill(char *msg){
    perror(msg);
    exit(1);
}
int main(int argc, char *argv[]){
    int sock, rv;
    struct sockaddr_in my_address, other_address;
    char buf[BUFSIZE];
    socklen_t addr_len = sizeof(struct sockaddr_in);
    fd_set set;
    struct timeval timeout;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1){
        kill("Socket error!");
    }

    memset((char *)&my_address, 0, addr_len); //zera a memoria
    my_address.sin_family = AF_INET;
    my_address.sin_port = htons(PORT);
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr*)&my_address, addr_len) == -1){
        kill("Bind error!");
    }
    memset(buf, 0, BUFSIZE);
    ssize_t received;
    printf("Aguardando...\n");

    FD_ZERO(&set); //limpa o set
    FD_SET(sock, &set); //Une o socket com o set
    timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
    timeout.tv_usec = 0;
    bool enable_send = true;

    while(enable_send){
    rv = select(sock + 1, &set, NULL, NULL, &timeout);

    if (rv == SOCKET_ERROR)
        kill("Socket error!");
    else if (rv == 0)
        printf("Timeout expired!");
    else{
        received = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&my_address, &addr_len);
        if (received == SOCKET_ERROR)
            kill("Error receiving!");
        else{
            if(received != BUFSIZE)
                enable_send = false;
            printf("Recebido com sucesso! %ld bytes!\n%s\n",received,buf);
        }
    }
}

    close(sock);
    return 0;
}
