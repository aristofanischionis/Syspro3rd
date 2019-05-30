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

int logOn(Node **headList, char *buffer, int sd, int max_clients, int **client_socket, int *max_sd, fd_set readfds, char **IP, int *port)
{
    char *IPaddr;
    int porta;
    int i = 0;
    int tempSD;
    char *message;
    struct sockaddr_in client_addr;
    message = malloc(256);
    IPaddr = malloc(20);
    sscanf(buffer, "LOG_ON < %s , %d >", IPaddr, &porta);
    printf("Server read at log on %s, %d\n", IPaddr, porta);
    int client = socket(AF_INET, SOCK_STREAM, 0);
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(IPaddr);
    client_addr.sin_port = htons(porta);

    strcpy(*IP, IPaddr);
    *port = porta;
    // connect
    int con = connect(client, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
    if (con == 0)
        printf("Client Connected\n");
    else
        printf("Error in Connection\n");

    // send(sd, "WELCOME", 9, 0);
    // now that I have a a new client update list
    // push it in the list, if t doesn't exist
    push(headList, IPaddr, porta);
    // change sd to client because it is the new socket to connect and send messages.
    for (i = 0; i < max_clients; i++)
    {
        //if position is sd's
        if ((*client_socket)[i] == sd)
        {
            (*client_socket)[i] = client;
            printf("Adding to list of sockets as %d\n", i);
            //if valid socket descriptor then add to read list
            if (client > 0)
                FD_SET(client, &readfds);

            //highest file descriptor number, need it for the select function
            if (client > *max_sd)
                *max_sd = client;
            break;
        }
    }

    // printf("The list currently consists of: \n");
    // printList(*headList);
    // now I have to send to all users in the list a message USER_ON
    sprintf(message, "USER_ON < %s , %d >", IPaddr, porta);
    printf("My message is : %s", message);

    // send it to all others
    for (i = 0; i < max_clients; i++)
    {
        tempSD = (*client_socket)[i];
        if (tempSD == client)
        {
            send(tempSD, "WELCOME", 8, 0);
            continue;
        }
        send(tempSD, message, strlen(message), 0);
    }

    return client;
}

void getClients(Node **headList, int sd, char *IP, int port)
{
    char *result;
    result = malloc(BUFSIZ);

    listToString(*headList, &result, IP, port);
    printf("My string is : %s\n", result);
    // now send it to this socket
    send(sd, result, strlen(result) + 1, 0);
    free(result);
    free(IP);
}

void logOff(Node **headList, char *buffer, int sd, int max_clients, int client_socket[])
{
    char *IP, *message;
    int port, tempSD, i;
    IP = malloc(20);
    message = malloc(50);
    sscanf(buffer, "LOG_OFF < %s , %d >", IP, &port);
    printf("LOG_OFF got from ---> , ip %s , port %d \n", IP, port);
    send(sd, "Ok byeee", 10, 0);
    if (deleteNode(headList, IP, port) == 0)
    {
        fprintf(stderr, "ERROR_IP_PORT_NOT_FOUND_IN_LIST\n");
    }
    sprintf(message, "USER_OFF < %s , %d >", IP, port);
    printf("My message is : %s\n", message);
    for (i = 0; i < max_clients; i++)
    {
        tempSD = client_socket[i];
        if (tempSD == sd)
        {
            send(tempSD, "JUST_IGNORE_THIS", 18, 0);
            continue;
        }
        send(tempSD, message, strlen(message), 0);
    }
}
