#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "../include/codepoly.h"


void *psend(void *arg)
{
    int proxysoc = *((int*)arg);
    struct sockaddr_in localadd;
    socklen_t localaddlen = sizeof(localadd);
    
    // recupere l'adresse ip et le port du client pour l'envoyer
    if(getsockname(proxysoc,(void*)&(localadd),&localaddlen)==-1)
        perror("error getsockname :");
    
    printf("port : %d\n",htons(localadd.sin_port));
    printf("ip : %s\n",inet_ntoa(localadd.sin_addr));

    uint16_t bufencoded[1024]; // 2 octets de port + 4 octets d'adresse + 1018 octets de message
    char buf[1024];

    // encode l'adresse ip et le port
    bufencoded[0] = encode(htons(localadd.sin_port),polynome);
    bufencoded[1] = encode(htons(localadd.sin_port)>>8,polynome);   
    bufencoded[2] = encode(localadd.sin_addr.s_addr,polynome);
    bufencoded[3] = encode(localadd.sin_addr.s_addr>>8,polynome);
    bufencoded[4] = encode(localadd.sin_addr.s_addr>>16,polynome);
    bufencoded[5] = encode(localadd.sin_addr.s_addr>>24,polynome);

    while(1)
    {
        int nb = read(0,buf,1018);
        if(nb == -1)
            perror("error read :");

        buf[nb-1]='\0'; //remove \n
        
        //encode message
        for(int i = 0;i<nb;i++)
        {
            bufencoded[i+6] = encode(buf[i],polynome);
        }
        
        // envoie le message
        if(send(proxysoc,&bufencoded,1024*sizeof(uint16_t),0)==-1)
        {
            perror("erreur send\n");
            exit(1);    
        }
    }
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("usage: %s <ip_proxy> <port_proxy>\n",argv[0]);
        exit(1);
    }
    
    int soc = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in proxyadd;
    proxyadd.sin_family = AF_INET;
    proxyadd.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(proxyadd.sin_addr));
    socklen_t plen = sizeof(proxyadd);

    if(connect(soc,(void*)&proxyadd,plen) == -1){perror("error connect \n");exit(1);}
    printf("Connected to proxy\n");

    psend(&soc);
    
    close(soc);
}