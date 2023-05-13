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
        if(code==0){printf("proxy disconnected\n");exit(0);}
        
        uint8_t table[16][2];
        tablerreur(polynome,table);
        
        //decode le port et l'adresse
        clientadd.sin_port = decode(bufencoded[0],polynome,table);
        clientadd.sin_port |= decode(bufencoded[1],polynome,table)<<8;
        printf("port : %d\n",clientadd.sin_port);
        clientadd.sin_addr.s_addr = decode(bufencoded[2],polynome,table);
        clientadd.sin_addr.s_addr |= decode(bufencoded[3],polynome,table)<<8;
        clientadd.sin_addr.s_addr |= decode(bufencoded[4],polynome,table)<<16;
        clientadd.sin_addr.s_addr |= decode(bufencoded[5],polynome,table)<<24;
        printf("ip : %s\n",inet_ntoa(clientadd.sin_addr));

        //decode le message
        for(int i = 6;i<code;i++)
        {
            buf[i-6] = decode(bufencoded[i],polynome,table);
            if(buf[i-6]=='\0')
                break;
        }
        printf("%s\n",buf);
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

    struct sockaddr_in add;
    add.sin_family = AF_INET;
    add.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(add.sin_addr));

    if(bind(soc,(void*)&add,sizeof(add)) == -1)
    {
        perror("error bind\n");
        exit(1);
    }
    
    int psoc;
    struct sockaddr_in peer;
    socklen_t plen;

    if(listen(soc,1)==-1)
    {
        perror("error listen\n");
        exit(1);
    }

    // connection from proxy
    if((psoc = accept(soc,(void*)&peer,&plen))==-1){perror("error accept\n");exit(1);}
    printf("Connected to proxy\n");

    while(1)
    {
        preceive(&psoc);
    }
    
    close(soc);
}
