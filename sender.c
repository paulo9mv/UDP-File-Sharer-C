#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include<sys/types.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */

#define BUFSIZE
#define PORT 8888
void kill(char *msg){
    perror(msg);
    exit(1);
}
unsigned long fsize(char* file)
{
    FILE * f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long)ftell(f);
    fclose(f);
    return len;
}
int main(){
    int sock, addr_len;
    unsigned long file_size;
    struct sockaddr_in my_address, other_address;
    FILE *fd;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1){
        kill("Socket error!");
    }

    addr_len = sizeof(struct sockaddr_in);

    memset((char *)&my_address, 0, addr_len); //zera a memoria
    my_address.sin_family = AF_INET;
    my_address.sin_port = htons(PORT);
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);


    fd = fopen("texto.txt","r");
    if(fd == NULL){
        kill("File not found!");
    }

    file_size = fsize("texto.txt");

    
    //char *buf = malloc(sizeof(char) *
    /*
        COdigo para tratar informaçao recebida
    */
    close(fd);
    close(sock);
    return 0;
}
