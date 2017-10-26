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

typedef int bool;
#define true 1
#define false 0
#define BUFSIZE 103
#define PORT 67
#define FILENAME_SIZE 100
#define PACKET_SIZE 100
#define IP_DESTINY "255.255.255.255"
#define SOCKET_ERROR -1
#define SOCKET_READ_TIMEOUT_SEC 2
#define FILENAME "texto.txt"

struct timer{
    double tempo;
};

unsigned long packets_to_send;

void kill(char *msg){
    perror(msg);
    exit(1);
}

unsigned long fsize(char* file)
{
    FILE * f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long)ftell(f);
    fclose(f);
    return len;
}
int copy_size(unsigned long file_size, int send_qtd){
    if(file_size > PACKET_SIZE * send_qtd)
        return PACKET_SIZE;
    else
        return PACKET_SIZE + file_size - (PACKET_SIZE*send_qtd);
}
int hasContent(){
    if(packets_to_send == 0)
        return 0;
    packets_to_send--;
    return 1;
}
unsigned long packets_counter(unsigned long file_size){
    return (file_size/PACKET_SIZE) + 1;
}
char ack(char atual){
    if (atual == '1')
        return '0';
    return '1';
}
int main(int argc, char *argv[]){
    int sock;
    socklen_t addr_len;
    unsigned long file_size, start = 0, size, packets_send = 0;
    char atual_ack = '1';
    struct sockaddr_in my_address, other_address;
    FILE *fd;
    char buffer[BUFSIZE];
    bool enable_send = true;
    int packs_to_expire = 0;
    ssize_t received;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1)
        kill("Socket error!");

    addr_len = sizeof(struct sockaddr_in);

    memset((char *)&my_address, 0, addr_len); //zera a memoria
    other_address.sin_family = AF_INET;
    other_address.sin_port = htons(PORT);
    other_address.sin_addr.s_addr = inet_addr(IP_DESTINY);

    my_address.sin_family = AF_INET;
    my_address.sin_port = htons(6000);
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr*)&my_address, addr_len) == -1){
        kill("Bind error!");
    }

    fd = fopen(FILENAME,"rb");

    if(fd == NULL)
        kill("File not found!");

    file_size = fsize(FILENAME);
    packets_to_send = packets_counter(file_size);

    char *file = malloc((sizeof(char) * file_size) + 1);

    if(file == NULL)
        kill("Memory error!");
    if(fread(file, 1, file_size, fd) != file_size)
        kill("Copy error!");

    file[file_size] = '\0';

    int rv;
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set); //limpa o set
    FD_SET(sock, &set); //Une o socket com o set

    printf("Iniciando transferencia...\n");
    timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    while(hasContent()){
        size = copy_size(file_size, packets_send + 1);

        atual_ack = ack(atual_ack);
        buffer[0] = atual_ack;

        if(size > 0)
            strncpy(buffer + 1, file + start, size);

        start += size;

        while(enable_send && packs_to_expire != 10){
            if(sendto(sock, buffer, (unsigned long) (size + 1), 0, (struct sockaddr*)&other_address, addr_len) == -1)
                kill("Send failed!");
            else
                printf("%ld bytes enviados com sucesso!\n", size + 1);

            timeout.tv_usec = 0;
            timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;


        rv = select(sock + 1, &set, NULL, NULL, &timeout);

        if (rv == SOCKET_ERROR)
            kill("Socket error!");
        else if (rv == 0){
            packs_to_expire++;
            printf("Timeout expired! %d try!\n", packs_to_expire);
        }
        else{
            received = recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr*)&my_address, &addr_len);
            packs_to_expire = 0;

            if (received == SOCKET_ERROR)
                kill("Error receiving!");
            else{
                enable_send = false;
                if(buffer[0] == (atual_ack + 2)){
                    enable_send = false;
                    printf("Ack %c recebido com sucesso!\n", buffer[0]);
                }
            }
        }
        }
        enable_send = true;
        if(packs_to_expire == 10){
            kill("No response! Stopping!");
        }
        memset(buffer, 0, BUFSIZE);
        packs_to_expire = 0;
        packets_send++;
    }

    if(fclose(fd) != 0)
        kill("Error during file closing!");

    if(close(sock) != 0)
        kill("Error during socket closing!");

    free(file);
    return 0;
}
