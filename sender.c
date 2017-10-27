#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include <arpa/inet.h>
#include<sys/types.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */
#include <time.h>

typedef int bool;
#define true 1
#define false 0

#define BUFSIZE 102
#define PACKET_SIZE 100

#define DEFAULT_PORT 3000
#define PORT 67

#define SOCKET_ERROR -1
#define SOCKET_READ_TIMEOUT_SEC 2
#define LIMITE 10

unsigned long packets_to_send;

//Mostra uma mensagem e encerra o programa
void kill(char *msg){
    perror(msg);
    exit(1);
}

//Determina o tamanho do arquivo em bytes
unsigned long fsize(char* file){
    FILE * f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long)ftell(f);
    fclose(f);
    return len;
}

//Determina quantos bytes deverao ser copiados
int copy_size(unsigned long file_size, int send_qtd){
    if(file_size > PACKET_SIZE * send_qtd)
        return PACKET_SIZE;
    else
        return PACKET_SIZE + file_size - (PACKET_SIZE*send_qtd);
}

//Checa se ha conteudo a ser enviado
int hasContent(){
    if(packets_to_send == 0)
        return 0;
    packets_to_send--;
    return 1;
}

//Determina quantos pacotes serao necessarios para o envio
unsigned long packets_counter(unsigned long file_size){
    return (file_size/PACKET_SIZE) + 1;
}
char ack(char atual){
    if (atual == '1')
        return '0';
    return '1';
}
int main(int argc, char *argv[]){

    if(argc != 4){
        kill("Missing arguments! <filename><ip><port>");
    }

    int sock, rv, packs_to_expire = 0, on = 1;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    unsigned long file_size, start = 0, size, packets_send = 0;
    char atual_ack = '1';
    struct sockaddr_in my_address, other_address;
    FILE *fd;
    char buffer[BUFSIZE];
    bool enable_send = true;
    ssize_t received;
    fd_set set;
    struct timeval timeout;

    //Inicializa o socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1)
        kill("Socket error!");

    //Da permissoes ao socket
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    //Zera a memoria de my_address e seta os parametros
    memset((char *)&my_address, 0, addr_len);
    my_address.sin_family = AF_INET;
    my_address.sin_port = htons(DEFAULT_PORT);
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);

    //Seta os parametros do destino
    other_address.sin_family = AF_INET;
    other_address.sin_port = htons(atoi(argv[3]));
    other_address.sin_addr.s_addr = inet_addr(argv[2]);

    //Realiza o bind do socket com a porta
    if(bind(sock, (struct sockaddr*)&my_address, addr_len) == -1){
        kill("Bind error!");
    }

    //Abre o arquivo e checa sua existencia
    fd = fopen(argv[1],"rb");
    if(fd == NULL)
        kill("File not found!");

    //Determina o tamanho do arquivo e qtos pacotes precisará p/ envio
    file_size = fsize(argv[1]);
    packets_to_send = packets_counter(file_size);

    //Aloca um buffer e copia os dados do arquivo para este buffer
    char *file = malloc((sizeof(char) * file_size) + 1);
    if(file == NULL)
        kill("Memory error!");
    if(fread(file, 1, file_size, fd) != file_size)
        kill("Copy error!");

    //Transforma em string para usar strncpy
    file[file_size] = '\0';

    FD_ZERO(&set); //limpa a variavel set para select()
    FD_SET(sock, &set); //Une o socket com o set

    printf("Iniciando transferencia...\n");

    //Seta os tempos de timeout
    timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
    timeout.tv_usec = 0;


    //Enquanto há conteudo para ser enviado, envie
    while(hasContent()){

        //Tamanho do buffer a ser enviado (0 - PACKET_SIZE)
        size = copy_size(file_size, packets_send + 1);

        //Atualiza o ack
        atual_ack = ack(atual_ack);
        buffer[0] = atual_ack;

        //Copia para o buffer de envio os dados correspondentes ao buffer do arquivo
        if(size > 0)
            strncpy(buffer + 1, file + start, size);

        //Incrementa o ponteiro para o buffer do arquivo
        start += size;


        //Enquanto não atingiu pacotes limites e deve ser enviado, envie
        while(enable_send && packs_to_expire != LIMITE){

            //Envia o conteudo do buffer para o destino
            if(sendto(sock, buffer, (unsigned long) (size + 1), 0, (struct sockaddr*)&other_address, addr_len) == -1)
                kill("Send failed!");

            //select() gera o timeout e confere o socket, caso haja dados chegando
            rv = select(sock + 1, &set, NULL, NULL, &timeout);


            if (rv == SOCKET_ERROR)     //Erro no socket
                kill("Socket error!");
            else if (rv == 0){          //Nada recebido

                //Incrementa a quantidade de pacotes para expirar
                packs_to_expire++;

                //Reseta os dados para select()
                timeout.tv_usec = 0;
                timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
                FD_ZERO(&set);
                FD_SET(sock, &set);

                printf("Timeout expired! %d try!\n", packs_to_expire);
            }
            else{    //Algo recebido

                //Erro durante recebimento (-1)
                if (recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr*)&my_address, &addr_len) == SOCKET_ERROR)
                    kill("Error receiving!");
                else{

                    //Confere se é o ACK correto
                    if(buffer[0] == (atual_ack + 2)){
                        enable_send = false;
                        printf("Ack %c recebido com sucesso!\n", buffer[0]);
                    }

                    //Caso contratio, incrementa o numero de tentativas
                    else{
                        packs_to_expire++;
                        printf("Not an ack! Sending again! %d try!\n", packs_to_expire);
                    }
                }
            }
        }

        //Seta enable_send para o proximo pacote
        enable_send = true;

        //Checa se atingiu o limite
        if(packs_to_expire == LIMITE){
            kill("No response! Stopping!");
        }

        //Limpa o buffer
        memset(buffer, 0, BUFSIZE);

        //Reseta parametros e timeout
        packs_to_expire = 0;
        packets_send++;
        timeout.tv_usec = 0;
        timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
    }


    if(fclose(fd) != 0)
        kill("Error during file closing!");

    if(close(sock) != 0)
        kill("Error during socket closing!");

    free(file);
    return 0;
}
