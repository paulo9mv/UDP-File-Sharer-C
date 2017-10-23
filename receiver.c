#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include<sys/types.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */

#define PORT 8888
void kill(char *msg){
    perror(msg);
    exit(1);
}
int main(int argc, char *argv[]){
    int sock, addr_len;
    struct sockaddr_in my_address, other_address;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1){
        kill("Socket error!");
    }

    addr_len = sizeof(struct sockaddr_in);

    memset((char *)&my_address, 0, addr_len); //zera a memoria
    my_address.sin_family = AF_INET;
    my_address.sin_port = htons(PORT);
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr*)&my_address, addr_len) == -1){
        kill("Bind error!");
    }

    /*
    CÓDIGO PARA TRATAR A INFORMAÇÃO RECEBIDA
    */

    close(sock);
    return 0;
}
