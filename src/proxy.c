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
    int ssoc;
    int csoc;
}soc2;

void *prelay(void *arg)
{
    int csoc = (((soc2*)arg)->csoc);
    int ssoc = (((soc2*)arg)->ssoc);
    uint16_t buf[1024];
    int code;
    srand(time(NULL));
    
    while((code=recv(csoc,buf,1024*sizeof(uint16_t),0)) != -1)
    {
        if(code==0){printf("client disconnected\n");exit(0);}
        
        // alteration message ~1/3 chance
        if(!(rand()%3))
        {
            //donne la taille du message
            int size = get_index(encode('\0',polynome),&buf[6],1024) + 6;
            int ind = rand()%size;
            buf[ind] = chg_nth_bit(rand()%8,buf[ind]);
            buf[ind] = chg_nth_bit(rand()%8,buf[ind]);
            printf("Message altered\n");
        }

        if(send(ssoc,buf,1024*sizeof(uint16_t),0) == -1){perror("erreur send\n");exit(1);}
    }
    
    perror("erreur recv\n");
    exit(1);
}

int main(int argc, char **argv)
{    
    if(argc != 5)
    {
        printf("usage: %s <ip> <port> <ipserver> <portserver>\n",argv[0]);
        exit(1);
    }
    (void)polynome;

    int soc = socket(AF_INET,SOCK_STREAM,0);
    int ssoc = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in add;
    add.sin_family = AF_INET;
    add.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(add.sin_addr));

    if(bind(soc,(void*)&add,sizeof(add)) == -1)
    {
        perror("error bind\n");
        exit(1);
    }

    add.sin_port = htons(atoi(argv[4]));
    inet_aton(argv[3], &(add.sin_addr));
    
    int csoc;
    struct sockaddr_in peer;
    socklen_t plen = sizeof(struct sockaddr_in);

    if(listen(soc,1)==-1)
    {
        perror("error listen\n");
        exit(1);
    }
    // connection to server
    if(connect(ssoc,(void*)&add,plen) == -1){perror("error connect \n");exit(1);}
    printf("Connected to server\n");
    
    // connection from client, 
    if((csoc = accept(soc,(void*)&peer,&plen))==-1){perror("error accept\n");exit(1);}
    printf("Connected to client\n");
    
    soc2 s;
    while(1)
    {
        s.csoc = csoc;
        s.ssoc = ssoc;
        prelay(&s);
    }
    
    close(soc);
}