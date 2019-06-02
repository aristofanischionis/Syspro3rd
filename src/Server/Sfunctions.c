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
#include "../HeaderFiles/Common.h"
#include "../HeaderFiles/LinkedList.h"

void logOn(Node **headList, char *buffer)
{
    char *IPaddr;
    int porta;
    char *message;
    message = malloc(BUFSIZ+1);
    IPaddr = malloc(20);
    sscanf(buffer, "LOG_ON < %s , %d >", IPaddr, &porta);
    
    int client = connect_to_socket(IPaddr, porta);
    int tempClientSD;
    // now I have to send to all users in the list a message USER_ON
    sprintf(message, "USER_ON < %s , %d >", IPaddr, porta);
    // printf("My message is : %s", message);
    send(client, "WELCOME", 8, 0);
    // close(client);
    // send it to all others
    Node *temp;
    temp = *headList;
    while (temp != NULL)
    {
        tempClientSD = connect_to_socket(temp->IP, temp->port);
        send(tempClientSD, message, strlen(message)+1, 0);
        close(tempClientSD);
        temp = temp->next;
    }

    // now that I have a a new client update list
    // push it in the list, if t doesn't exist
    push(headList, IPaddr, porta);

    recv(client, message, BUFSIZ, 0);
    if (!strcmp(message, "GET_CLIENTS"))
    {
        getClients(headList, client, IPaddr, porta);
    }
    else
    {
        fprintf(stderr, "something went wrong in receiving getclients\n");
    }
    close(client);
}

void getClients(Node **headList, int sd, char *IP, int port)
{
    char *result;
    result = malloc(BUFSIZ);

    listToString(*headList, &result, IP, port);
    // printf("My string is : %s\n", result);
    // now send it to this socket
    send(sd, result, strlen(result) + 1, 0);
    free(result);
    free(IP);
}

void logOff(Node **headList, char *buffer, int sd)
{
    char *IP, *message;
    int port;
    IP = malloc(20);
    message = malloc(50);
    sscanf(buffer, "LOG_OFF < %s , %d >", IP, &port);
    // printf("LOG_OFF got from ---> , ip %s , port %d \n", IP, port);
    // send(sd, "Ok byeee", 10, 0);
    // close(sd);
    if (deleteNode(headList, IP, port) == 0)
    {
        fprintf(stderr, "ERROR_IP_PORT_NOT_FOUND_IN_LIST\n");
        exit(1);
    }
    sprintf(message, "USER_OFF < %s , %d >", IP, port);
    Node *temp;
    temp = *headList;
    int tempClientSD;
    while (temp != NULL)
    {
        tempClientSD = connect_to_socket(temp->IP, temp->port);
        send(tempClientSD, message, strlen(message)+1, 0);
        close(tempClientSD);
        temp = temp->next;
    }
    
}
