#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

typedef struct
{
    int ssoc;
    int csoc;
}soc2;

char chg_nth_bit(int n, char m)//15-0
{
    char maske = 0b00000001;
    return (maske <<= n) ^ m;
}

void *prelay(void *arg)
{
    int csoc = (((soc2*)arg)->csoc);
    int ssoc = (((soc2*)arg)->ssoc);
    char buf[1024];
    int code;
    srand(time(NULL));
    
    while((code=recv(csoc,buf,1024,0)) != -1)
    {
        if(code==0){printf("client disconnected\n");exit(0);}
        printf("Incoming message : %s\n",buf);
        
        // alteration message
        if(!(rand()%3))
        {
            int ind = rand()%strlen(buf);
            buf[ind] = chg_nth_bit(rand()%8,buf[ind]);
            printf("Alterated message : %s\n",buf);
        }

        if(send(ssoc,buf,1024,0) == -1){perror("erreur send\n");exit(1);}
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