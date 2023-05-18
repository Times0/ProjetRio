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
    int serversoc;
    int clientsoc;
} soc2;

/* Corrupt message with a probability of 1/proba 1 time
 * 2 times with a probability of 1/proba²
 * Corruption is done by changing 1 or 2 bits in the message in the same byte
 */
void corrupt_message(uint16_t *buf, int proba)
{
    int times = 0;
    if (!(rand() % proba))
    {
        int size = get_index(encode('\0', polynom), &buf[7], 1024);
        if (size != 0)
        {
            times++;
            int ind = rand() % size + 7;
            int nbit = rand() % 16;
            buf[ind] = chg_nth_bit(nbit, buf[ind]);

            if (!(rand() % proba))
            {
                buf[ind] = chg_nth_bit((nbit + 1) % 16, buf[ind]);
                times++;
            }
        }
    }
    printf("Message corrupted %d time(s)\n", times);
}

// One relay fonction for each client
void *prelay(void *arg)
{
    int clientsoc = (((soc2 *)arg)->clientsoc);
    int serversoc = (((soc2 *)arg)->serversoc);
    uint16_t buf[1024];
    int code;
    srand(time(NULL));

    while ((code = recv(clientsoc, buf, 1024 * sizeof(uint16_t), 0)) != 0)
    {
        if (code == -1)
        {
            perror("recv\n");
            exit(1);
        }

        corrupt_message(buf, 2);

        // send to server
        CHK(send(serversoc, buf, 1024 * sizeof(uint16_t), 0));
    }

    printf("Client disconnected\n");
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        printf("usage: %s <ip_proxy> <port_proxy> <ip_server> <port_server>\n", argv[0]);
        exit(1);
    }

    int soc = socket(AF_INET, SOCK_STREAM, 0);
    int serversoc = socket(AF_INET, SOCK_STREAM, 0);

    if (soc == -1 || serversoc == -1)
    {
        perror("error socket \n");
        exit(1);
    }

    // Set SO_REUSEADDR option
    int reuseAddr = 1;
    if (setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0)
    {
        perror("Setting SO_REUSEADDR failed");
        exit(EXIT_FAILURE);
    }

    // set up local address
    struct sockaddr_in localadd;
    localadd.sin_family = AF_INET;
    localadd.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(localadd.sin_addr));

    CHK(bind(soc, (struct sockaddr *)&localadd, sizeof(localadd)));

    // set up server address
    struct sockaddr_in serveradd;
    serveradd.sin_family = AF_INET;
    serveradd.sin_port = htons(atoi(argv[4]));
    inet_aton(argv[3], &(serveradd.sin_addr));

    int clientsoc;
    struct sockaddr_in clientadd;
    socklen_t len = sizeof(struct sockaddr_in);

    printf("============================ Proxy (read only) ============================\n");

    CHK(listen(soc, 1));

    // connection to server
    if (connect(serversoc, (void *)&serveradd, len) == -1)
    {
        perror("error connect \n");
        exit(1);
    }
    printf("Connected to server\n");

    soc2 s;
    // loop to accept as many clients as we want
    while (1)
    {
        // connection from client,
        CHK(clientsoc = accept(soc, (struct sockaddr *)&clientadd, &len));
        printf("Connected to client on ip: %s:%d\n", inet_ntoa(clientadd.sin_addr), ntohs(clientadd.sin_port));
        s.clientsoc = clientsoc;
        s.serversoc = serversoc;

        // if 2 clients connect at the same time s will be overwritten
        pthread_t thread;
        TCHK(pthread_create(&thread, NULL, prelay, &s));
    }

    CHK(close(soc));
}