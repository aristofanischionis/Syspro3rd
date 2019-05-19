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

void logOn(Node **headList, char *buffer, int sd, int max_clients, int client_socket[])
{
    printf("I am log on\n");
    char *IP;
    int port;
    int i = 0, x = 0;
    int tempSD;
    char *message;
    message = malloc(256);
    IP = malloc(20);
    sscanf(buffer, "LOG_ON < %s , %d >", IP, &port);

    printf("Server read at log on %s, %d\n", IP, port);
    send(sd, "I received your message", 25, 0);
    // now that I have a a new client update list
    // push it in the list, if t doesn't exist
    push(headList, IP, port);
    // printf("The list currently consists of: \n");
    // printList(*headList);
    // now I have to send to all users in the list a message USER_ON
    sprintf(message, "USER_ON < %s , %d >\n", IP, port);
    printf("My message is : %s", message);
    // count objects
    x = countNodes(*headList);
    for (i = 0; i < max_clients; i++)
    {
        tempSD = client_socket[i];
        if (tempSD == sd)
        {
            continue;
        }
        send(tempSD, message, strlen(message), 0);
    }
}

void getClients(Node **headList, int sd)
{
    char *result;
    result = malloc(BUFSIZ);
    listToString(headList, result);
    printf("::::::::::::This is getClients::::::::::::::::\n");
    printf("My string is : %s\n", result);
    // now send it to this socket
    send(sd, result, strlen(result) + 1, 0);
}

void logOff(Node **headList, int sd, int max_clients, int client_socket[])
{
    struct sockaddr_in address;
    int addrlen;
    char *IP, *message;
    int port, tempSD, x, i;
    IP = malloc(20);
    message = malloc(50);
    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    printf("LOGOFF got from ---> , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    send(sd, "Ok byeee", 10, 0);
    strcpy(IP, inet_ntoa(address.sin_addr));
    port = ntohs(address.sin_port);
    if (deleteNode(headList, IP, port) == 0)
    {
        fprintf(stderr, "ERROR_IP_PORT_NOT_FOUND_IN_LIST\n");
    }
    sprintf(message, "USER_OFF < %s , %d >\n", IP, port);
    printf("My message is : %s\n", message);
    // count objects
    x = countNodes(*headList);
    for (i = 0; i < max_clients; i++)
    {
        tempSD = client_socket[i];
        if (tempSD == sd)
        {
            continue;
        }
        send(tempSD, message, strlen(message), 0);
    }
}