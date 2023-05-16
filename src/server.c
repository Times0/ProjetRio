#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "../include/codepoly.h"

typedef struct
{
    struct sockaddr_in addr;
    uint8_t numpacket;
}arg_retransmit;

void *retransmit(void *arg)
{
    // copy of arguments
    struct sockaddr_in clientadd = ((arg_retransmit*)arg)->addr;
    uint8_t numpacket = ((arg_retransmit*)arg)->numpacket;
    
    int soc = socket(AF_INET,SOCK_STREAM,0);
    socklen_t len = sizeof(clientadd);
    
    // connect to client in order to retransmit
    if(connect(soc,(void*)&clientadd,len) == -1){perror("error connect \n");exit(1);}
    
    // send numpacket to client
    uint16_t encodednumpacket = encode(numpacket,polynom);
    if(send(soc,&encodednumpacket,sizeof(uint16_t),0)==-1){perror("erreur send\n");exit(1);}

    close(soc);
    return NULL;
}

void *preceive(void *arg)
{
    int psoc = *((int*)arg);
    int code, error = 0;
    struct sockaddr_in clientadd;
    clientadd.sin_family = AF_INET;
    uint16_t bufencoded[1024];
    char buf[1024];

    // initialisation table error-correction
    uint8_t table[16][2];
    initerrortable(polynom,table);

    while(1)
    {
        error = 0;
        code=recv(psoc,bufencoded,1024*sizeof(uint16_t),0);
        
        if(code==0){printf("Proxy disconnected\n");exit(0);}
        if(code==-1){perror("erreur recv\n");exit(1);}
        
        //decode message
        for(int i = 0;i<code;i++)
        {
            buf[i] = decode(bufencoded[i],polynom,table);
            //if decode failed
            if(buf[i]==-1)
                error = 1;
            //if end of message, i > 6 in case of bytes of port, ip address or numpacket is 0
            if(buf[i]=='\0' && i>6)
                break;
        }

        clientadd.sin_port = (uint8_t)buf[0];
        clientadd.sin_port |= (uint8_t)buf[1]<<8;
        clientadd.sin_port = htons(clientadd.sin_port);
        clientadd.sin_addr.s_addr = (uint8_t)buf[2];
        clientadd.sin_addr.s_addr |= (uint8_t)buf[3]<<8;
        clientadd.sin_addr.s_addr |= (uint8_t)buf[4]<<16;
        clientadd.sin_addr.s_addr |= (uint8_t)buf[5]<<24;
        
        if(error)
        {
            printf("Retransmission asked\n");
            pthread_t pid;
            arg_retransmit arg_r;
            arg_r.numpacket = buf[6];
            arg_r.addr = clientadd;
            pthread_create(&pid,NULL,retransmit,&arg_r);
        }
        else
        {
            printf("%s\n",&buf[7]);
        }
    }  
}

int main(int argc, char **argv)
{    
    if(argc != 3)
    {
        printf("usage: %s <ip> <port>\n",argv[0]);
        exit(1);
    }
    
    int soc = socket(AF_INET,SOCK_STREAM,0);

    //local address (server)
    struct sockaddr_in localadd;
    localadd.sin_family = AF_INET;
    localadd.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(localadd.sin_addr));

    if(bind(soc,(void*)&localadd,sizeof(localadd)) == -1){perror("error bind\n");exit(1);}
    
    int psoc;
    struct sockaddr_in proxyadd;
    socklen_t len;

    if(listen(soc,1)==-1){perror("error listen\n");exit(1);}

    // connection from proxy
    if((psoc = accept(soc,(void*)&proxyadd,&len))==-1){perror("error accept\n");exit(1);}
    printf("Connected to proxy\n");

    preceive(&psoc);
}
