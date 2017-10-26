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
#define IP_DESTINY "255.255.255.255"
#define SOCKET_ERROR -1
#define SOCKET_READ_TIMEOUT_SEC 2
#define DEFAULT_FILE_NAME "new_file"

void kill(char *msg){
    perror(msg);
    exit(1);
}
char ack(char atual){
    if (atual == '1')
        return '0';
    return '1';
}
int main(int argc, char *argv[]){

    if(argc != 2){
        kill("Missing arguments! Dest port!");
    }

    int sock, rv;
    struct sockaddr_in my_address, other_address;
    char buf[BUFSIZE];
    socklen_t addr_len = sizeof(struct sockaddr_in);
    fd_set set;
    struct timeval timeout;
    char ack_flag[1];
    char atual_ack = '0';
    FILE *fd;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1){
        kill("Socket error!");
    }

int on=1;
setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    memset((char *)&my_address, 0, addr_len); //zera a memoria
    my_address.sin_family = AF_INET;
    my_address.sin_port = htons(atoi(argv[1]));
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr*)&my_address, addr_len) == -1){
        kill("Bind error!");
    }
    memset(buf, 0, BUFSIZE);
    ssize_t received;
    printf("Aguardando...\n");

    FD_ZERO(&set); //limpa o set
    FD_SET(sock, &set); //Une o socket com o set

    bool enable_send = true;
    bool first_pack = true;
    timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    while(enable_send){
            received = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&my_address, &addr_len);
            if(first_pack){
                printf("Checando a existencia do arquivo!\n");
                fd = fopen(DEFAULT_FILE_NAME, "r");
                if(fd == NULL){
                    fd = fopen(DEFAULT_FILE_NAME, "w");
                }
                else
                    kill("File already exists! Aborting!");
                first_pack = false;
            }
            if (received == SOCKET_ERROR)
                kill("Error receiving!");
            else{
                if(received != BUFSIZE)
                    enable_send = false;
            printf("Recebido com sucesso! %ld bytes!\n%s\n",received,buf);
                other_address.sin_family = AF_INET;
                other_address.sin_port = htons(ntohs(my_address.sin_port));
                other_address.sin_addr.s_addr = inet_addr(IP_DESTINY);
                if(buf[0] == atual_ack){
                    atual_ack = ack(atual_ack);
                    fwrite(buf + 1, received - 1, 1, fd);
                }

                ack_flag[0] = buf[0] + 2;

                if(sendto(sock, ack_flag,(unsigned long) 1, 0, (struct sockaddr*)&other_address, addr_len) == -1){
                    kill("Ack response failed!\n");
                }
                else{
                    printf("Ack respondido!\n");
                }

        }
    }

    fclose(fd);
    close(sock);
    return 0;
}
