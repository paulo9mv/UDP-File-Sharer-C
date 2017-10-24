#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include <arpa/inet.h>
#include<sys/types.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */

#define BUFSIZE 103
#define PORT 68
#define FILENAME_SIZE 100
#define PACKET_SIZE 100

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
    packets_to_send--;

    if(packets_to_send == 0)
        return 0;
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
void enviar(char datagram[]){
    //enviar com sendto
    printf("Enviou\n");
}
int main(int argc, char *argv[]){
    int sock;
    socklen_t addr_len;
    unsigned long file_size, start = 0, size, packets_send = 0;
    char atual_ack = '1';
    struct sockaddr_in my_address, other_address;
    FILE *fd;
    char buffer[BUFSIZE];

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1)
        kill("Socket error!");

    addr_len = sizeof(struct sockaddr_in);

    memset((char *)&my_address, 0, addr_len); //zera a memoria
    other_address.sin_family = AF_INET;
    other_address.sin_port = htons(PORT);
    other_address.sin_addr.s_addr = inet_addr("127.0.0.1");


    fd = fopen("texto.txt","rb");
    if(fd == NULL)
        kill("File not found!");

    file_size = fsize("texto.txt");
    packets_to_send = packets_counter(file_size);

    char *file = malloc((sizeof(char) * file_size) + 1);

    if(file == NULL)
        kill("Memory error!");
    if(fread(file, 1, file_size, fd) != file_size)
        kill("Copy error!");

    file[file_size] = '\0';

    while(hasContent()){
        size = copy_size(file_size, packets_send + 1);

        atual_ack = ack(atual_ack);
        buffer[0] = atual_ack;

        if(size > 0)
            strncpy(buffer + 1, file + start, size);

        start += size;

        if(sendto(sock, buffer, (unsigned long) (size + 1), 0, (struct sockaddr*)&other_address, addr_len) == -1)
            kill("Send failed!");
        else
            printf("Sucessful sent %ld bytes!\n", size+1);

        memset(buffer, 0, BUFSIZE);

        packets_send++;
    }
    /*
        COdigo para tratar informaçao recebida
    */
    if (fclose(fd) != 0)
        kill("Error during file closing!");

    if(close(sock) != 0)
        kill("Error during socket closing!");

    free(file);
    return 0;
}
