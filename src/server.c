#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "../include/codepoly.h"


void *preceive(void *arg)
{
    int psoc = *((int*)arg);
    int code;
    struct sockaddr_in clientadd;
    uint16_t bufencoded[1024];
    char buf[1024];
    while((code=recv(psoc,bufencoded,1024*sizeof(uint16_t),0)) != -1)
    {
        if(code==0){printf("Proxy disconnected\n");exit(0);}
        
        uint8_t table[16][2];
        tablerreur(polynome,table);
        
        //decode le message
        for(int i = 0;i<code;i++)
        {
            buf[i] = decode(bufencoded[i],polynome,table);
            
            //si decode a echoué
            if(buf[i]==6)
            {
                printf("Retransmission %d\n",buf[i]);
            }
            
            //si fin de message, i > 5 car on veut recuperer l'adresse ip et le port 
            if(buf[i]=='\0' && i>5)
                break;
        }

        clientadd.sin_port = (uint8_t)buf[0];
        clientadd.sin_port |= (uint8_t)buf[1]<<8;
        clientadd.sin_addr.s_addr = (uint8_t)buf[2];
        clientadd.sin_addr.s_addr |= (uint8_t)buf[3]<<8;
        clientadd.sin_addr.s_addr |= (uint8_t)buf[4]<<16;
        clientadd.sin_addr.s_addr |= (uint8_t)buf[5]<<24;
        
        printf("port : %d\n",clientadd.sin_port);
        printf("ip : %s\n",inet_ntoa(clientadd.sin_addr));
        printf("%s\n",&buf[6]);
    }
    perror("erreur recv\n");
    exit(1);

}

int main(int argc, char **argv)
{    
    if(argc != 3)
    {
        printf("usage: %s <ip> <port>\n",argv[0]);
        exit(1);
    }
    
    int soc = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in localadd;
    localadd.sin_family = AF_INET;
    localadd.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(localadd.sin_addr));

    if(bind(soc,(void*)&localadd,sizeof(localadd)) == -1)
    {
        perror("error bind\n");
        exit(1);
    }
    
    int psoc;
    struct sockaddr_in proxyadd;
    socklen_t len;

    if(listen(soc,1)==-1)
    {
        perror("error listen\n");
        exit(1);
    }

    // connection from proxy
    if((psoc = accept(soc,(void*)&proxyadd,&len))==-1){perror("error accept\n");exit(1);}
    printf("Connected to proxy\n");

    while(1)
    {
        preceive(&psoc);
    }
    
    close(soc);
}
