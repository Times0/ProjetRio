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
    int serversoc;
    int clientsoc;
}soc2;

void *prelay(void *arg)
{
    int clientsoc = (((soc2*)arg)->clientsoc);
    int serversoc = (((soc2*)arg)->serversoc);
    uint16_t buf[1024];
    int code,times;
    srand(time(NULL));
    
    while((code=recv(clientsoc,buf,1024*sizeof(uint16_t),0)) != 0)
    {
        if(code==-1){perror("erreur recv\n");exit(1);}
        
        times=0;
        // alteration message ~1/2 chance
        if(!(rand()%2))
        {
            // length of message
            int size = get_index(encode('\0',polynom),&buf[7],1024);
            //we only corrupt bits of message
            if(size != 0)
            {
                times++;
                int ind = rand()%size+7;
                int nbit = rand()%16;
                // one error correctable by server
                buf[ind] = chg_nth_bit(nbit,buf[ind]);
                // 1/4 chance to corrupt the message 2 times, not correctable by server
                if(!(rand()%2))
                {
                    buf[ind] = chg_nth_bit((nbit+1)%16,buf[ind]);
                    times++;
                }
                printf("Message corrupted %d time(s)\n",times);
            }
        }
        // send to server
        if(send(serversoc,buf,1024*sizeof(uint16_t),0) == -1){perror("erreur send\n");exit(1);}
    }
    
    printf("Client disconnected\n");
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{    
    if(argc != 5)
    {
        printf("usage: %s <ip_proxy> <port_proxy> <ip_server> <port_server>\n",argv[0]);
        exit(1);
    }

    int soc = socket(AF_INET,SOCK_STREAM,0);
    int serversoc = socket(AF_INET,SOCK_STREAM,0);

    // set up local address
    struct sockaddr_in localadd;
    localadd.sin_family = AF_INET;
    localadd.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(localadd.sin_addr));

    if(bind(soc,(void*)&localadd,sizeof(localadd)) == -1){perror("error bind\n");exit(1);}

    // set up server address
    struct sockaddr_in serveradd;
    serveradd.sin_port = htons(atoi(argv[4]));
    inet_aton(argv[3], &(serveradd.sin_addr));
    
    int clientsoc;
    struct sockaddr_in clientadd;
    socklen_t len = sizeof(struct sockaddr_in);

    if(listen(soc,1)==-1){perror("error listen\n");exit(1);}

    // connection to server
    if(connect(serversoc,(void*)&serveradd,len) == -1){perror("error connect \n");exit(1);}
    printf("Connected to server\n");
    
    soc2 s;
    while(1)
    {
        // connection from client, 
        if((clientsoc = accept(soc,(void*)&clientadd,&len))==-1){perror("error accept\n");exit(1);}
        printf("Connected to client\n");
        s.clientsoc = clientsoc;
        s.serversoc = serversoc;
        
        // if 2 clients connect at the same time s overwrited
        pthread_t thread;
        pthread_create(&thread,NULL,prelay,&s);
    }
    
    close(soc);
}