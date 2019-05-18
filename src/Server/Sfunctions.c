#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "headerfile.h"
#include "../HeaderFiles/LinkedList.h"

void logOn(Node *headList, char *buffer, int sd)
{
    printf("I am log on\n");
    char *IP;
    int port;
    IP = malloc(20);
    sscanf(buffer, "LOG_ON < %s , %d >", IP, &port);

    printf("Server read at log on %s, %d\n", IP, port);
    send(sd, "I received your message", 25, 0);
}

void getClients()
{
}

void logOff()
{
}