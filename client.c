
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

void *psend(void *arg)
{
    int psoc = *((int*)arg);
    char buf[1024];
    while(1)
    {
        int nb = read(0,buf,1024);
        buf[nb-1]='\0'; //remove \n
        if(send(psoc,buf,1024,0)==-1)
        {
            printf("hi");
            perror("erreur send\n");
            exit(1);    
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