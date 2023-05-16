#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "../include/codepoly.h"

#define TAILLE_PAQUET 1024

typedef struct
{
    int socproxy;
    int socserver;
    uint16_t historique[256][TAILLE_PAQUET];
}arg_t;

void tabcopy(uint16_t dest[],uint16_t src[],int size)
{
    for(int i = 0;i<size;i++)
        dest[i] = src[i];
}

void *psend(void *arg)
{
    arg_t *a = (arg_t*)arg;
    struct sockaddr_in localadd;
    socklen_t localaddlen = sizeof(localadd);
    uint8_t numpacket = 0;
    
    // get ip address and port of socket client-server    
    if(getsockname(a->socserver,(void*)&(localadd),&localaddlen)==-1){perror("error getsockname :");exit(1);}
    // printf("port : %d\n",htons(localadd.sin_port));
    // printf("ip : %s\n",inet_ntoa(localadd.sin_addr));

    uint16_t bufencoded[TAILLE_PAQUET]; // 2 bytes for port + 4 bytes for ip address + 1 byte number packet + 1017 bytes of message
    char buf[TAILLE_PAQUET];

    // encode ip address and port
    bufencoded[0] = encode(htons(localadd.sin_port),polynom);
    bufencoded[1] = encode(htons(localadd.sin_port)>>8,polynom);   
    bufencoded[2] = encode(localadd.sin_addr.s_addr,polynom);
    bufencoded[3] = encode(localadd.sin_addr.s_addr>>8,polynom);
    bufencoded[4] = encode(localadd.sin_addr.s_addr>>16,polynom);
    bufencoded[5] = encode(localadd.sin_addr.s_addr>>24,polynom);
    bufencoded[6] = encode(numpacket,polynom);

    while(1)
    {
        int nb = read(0,buf,1017);
        if(nb == -1){perror("error read :");exit(1);}

        buf[nb-1]='\0'; //remove \n
        
        //encode message
        for(int i = 0;i<nb;i++)
            bufencoded[i+7] = encode(buf[i],polynom);
        
        // send packet to proxy
        if(send(a->socproxy,&bufencoded,TAILLE_PAQUET*sizeof(uint16_t),0)==-1){perror("erreur send\n");exit(1);}

        // stocke packet
        tabcopy(a->historique[numpacket],bufencoded,1024);       
        // number packet, %256
        numpacket++;
        bufencoded[6] = encode(numpacket,polynom);
    }
}

// receive NAK from server
void *preceive(void *arg)
{
    arg_t *a = (arg_t*)arg;
    
    struct sockaddr_in serveradd;
    socklen_t len;
    int lsoc;
    uint16_t encodednumpacket;
    uint8_t table[16][2],numpacket;
    initerrortable(polynom,table);

    while(1)
    {
        // connection from server
        if((lsoc = accept(a->socserver,(void*)&serveradd,&len))==-1){perror("error accept\n");exit(1);}
        // get numpacket to retransmit
        if(recv(lsoc,&encodednumpacket,sizeof(uint16_t),0)==-1){perror("erreur recv\n");exit(1);}
        
        numpacket = decode(encodednumpacket,polynom,table);
        // resend message
        printf("Resend packet n°%d\n",numpacket);
        if(send(a->socproxy,a->historique[numpacket],1024*sizeof(uint16_t),0)==-1){perror("erreur send\n");exit(1);}
    }
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("usage: %s <ip_proxy> <port_proxy>\n",argv[0]);
        exit(1);
    }
    arg_t arg;    
    int socproxy = socket(AF_INET,SOCK_STREAM,0);
    int socserver = socket(AF_INET,SOCK_STREAM,0);

    // proxy address
    struct sockaddr_in proxyadd;
    proxyadd.sin_family = AF_INET;
    proxyadd.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(proxyadd.sin_addr));
    socklen_t plen = sizeof(proxyadd);

    // local address of socket client-serveur
    struct sockaddr_in localadd;
    localadd.sin_addr = proxyadd.sin_addr;
    localadd.sin_family = AF_INET;
    if(bind(socserver,(void*)&localadd,plen)==-1){perror("error bind:");exit(1);}
    if(listen(socserver,1)==-1){perror("error listen\n");exit(1);}

    // connexion to proxy
    if(connect(socproxy,(void*)&proxyadd,plen) == -1){perror("error connect \n");exit(1);}
    printf("Connected to proxy\n");
    
    arg.socproxy = socproxy;
    arg.socserver = socserver;
    pthread_t tid[2];
    pthread_create(&tid[0],NULL,psend,&arg);
    pthread_create(&tid[1],NULL,preceive,&arg);
    
    pthread_join(tid[0],NULL);
    pthread_join(tid[1],NULL);
    close(socproxy);
}