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
/*  Print  error  message  and  exit  */
void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void *Mainthread(void *args)
{
    // Two buffer are for message communication
    struct args_MainThread *arguments;
    char *clientIP;
    clientIP = malloc(20);
    strcpy(clientIP, "127.0.0.1");
    arguments = (struct args_MainThread *)args;
    char *message;
    message = malloc(BUFSIZ+1);
    struct sockaddr_in my_addr, my_addr1;
    int client = socket(AF_INET, SOCK_STREAM, 0);

    if (setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror_exit("setsockopt(SO_REUSEADDR) failed");

    if (client < 0)
        printf("Error in client creating\n");
    else
        printf("Client Created\n");

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(arguments->serverPort);

    // This ip address will change according to the machine
    my_addr.sin_addr.s_addr = inet_addr(arguments->serverIP);

    // binding client with that port
    my_addr1.sin_family = AF_INET;
    my_addr1.sin_addr.s_addr = INADDR_ANY;
    my_addr1.sin_port = htons(arguments->clientPort);

    // This ip address will change according to the machine
    my_addr1.sin_addr.s_addr = inet_addr(clientIP);

    printf("clientip %s, clientport %d,  serverip %s, serverport %d \n", inet_ntoa(my_addr1.sin_addr), ntohs(my_addr1.sin_port), inet_ntoa(my_addr.sin_addr), ntohs(my_addr.sin_port));
    if (bind(client, (struct sockaddr *)&my_addr1, sizeof(struct sockaddr_in)) == 0)
        printf("Binded Correctly\n");
    else
        printf("Unable to bind\n");
    
    int con = connect(client, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
    if (con == 0)
        printf("Client Connected\n");
    else
        printf("Error in Connection\n");

    sprintf(message, "LOG_ON < %s , %d >", inet_ntoa(my_addr1.sin_addr), ntohs(my_addr1.sin_port));
    send(client, message, BUFSIZ, 0);
    recv(client, message, BUFSIZ, 0);
    printf("Server : %s", message);
    close(client);
    return NULL;
}
