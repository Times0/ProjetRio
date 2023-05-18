#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "../include/codepoly.h"

#include <stdarg.h>
#include <unistd.h>
#include <stdnoreturn.h>

// Quelques macros pour se simplifier la vie
#define CHK(op)            \
    do                     \
    {                      \
        if ((op) == -1)    \
            raler(1, #op); \
    } while (0)
#define CHKN(op)           \
    do                     \
    {                      \
        if ((op) == NULL)  \
            raler(1, #op); \
    } while (0)

// Cette macro modifie errno avant d'appeler perror => pour les fct pthread_*
#define TCHK(op)                \
    do                          \
    {                           \
        if ((errno = (op)) > 0) \
            raler(1, #op);      \
    } while (0)

noreturn void raler(int syserr, const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    if (syserr == 1)
        perror("");

    exit(1);
}

/*_____________________________Début__________________________________________*/

typedef struct
{
    struct sockaddr_in addr;
    uint8_t numpacket;
} arg_retransmit;

void *retransmit(void *arg)
{
    // copy of arguments
    struct sockaddr_in clientadd = ((arg_retransmit *)arg)->addr;
    uint8_t numpacket = ((arg_retransmit *)arg)->numpacket;

    int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc == -1)
        raler(1, "socket");

    socklen_t len = sizeof(clientadd);

    // connect to client in order to retransmit
    CHK(connect(soc, (struct sockaddr *)&clientadd, len));

    // send numpacket to client
    uint16_t encodednumpacket = encode(numpacket, polynom);
    CHK(send(soc, &encodednumpacket, sizeof(uint16_t), 0));

    CHK(close(soc));
    return NULL;
}

void *preceive(void *arg)
{
    int psoc = *((int *)arg);
    int code, error = 0;
    struct sockaddr_in clientadd;
    clientadd.sin_family = AF_INET;
    uint16_t bufencoded[1024];
    char buf[1024];

    // initialisation table error-correction
    uint8_t table[16][2];
    initerrortable(polynom, table);

    while (1)
    {
        error = 0;
        CHK(code = recv(psoc, bufencoded, sizeof(bufencoded), 0));

        if (code == 0)
        {
            printf("Client (likely the proxy), disoconnected\n");
            CHK(close(psoc));
            exit(0);
        }

        // decode message
        for (int i = 0; i < code; i++)
        {
            buf[i] = decode(bufencoded[i], polynom, table);
            // if decode failed
            if (buf[i] == -1)
                error = 1;
            // if end of message, i > 6 in case of bytes of port, ip address or numpacket is 0
            if (buf[i] == '\0' && i > 6)
                break;
        }

        clientadd.sin_port = (uint8_t)buf[0];
        clientadd.sin_port |= (uint8_t)buf[1] << 8;
        clientadd.sin_port = htons(clientadd.sin_port);
        clientadd.sin_addr.s_addr = (uint8_t)buf[2];
        clientadd.sin_addr.s_addr |= (uint8_t)buf[3] << 8;
        clientadd.sin_addr.s_addr |= (uint8_t)buf[4] << 16;
        clientadd.sin_addr.s_addr |= (uint8_t)buf[5] << 24;

        if (error)
        {
            printf("Message has been corrupted, asking for retransmission...\n");
            pthread_t pid;
            arg_retransmit arg_r;
            arg_r.numpacket = buf[6];
            arg_r.addr = clientadd;
            TCHK(pthread_create(&pid, NULL, retransmit, &arg_r));
        }
        else
        {
            printf("> %s\n", &buf[7]);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int soc = socket(AF_INET, SOCK_STREAM, 0);

    // local address (server)
    struct sockaddr_in localadd;
    localadd.sin_family = AF_INET;
    localadd.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(localadd.sin_addr));

    CHK(bind(soc, (struct sockaddr *)&localadd, sizeof(localadd)));

    printf("============================ SERVER (read only) ============================\n");

    int psoc;
    struct sockaddr_in proxyadd;
    socklen_t len;

    CHK(listen(soc, 1));
    CHK(psoc = accept(soc, (struct sockaddr *)&proxyadd, &len));

    printf("Connected to proxy\n");
    preceive(&psoc);
}
