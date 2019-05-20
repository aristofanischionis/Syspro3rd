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
#include <errno.h>
#include "headerfile.h"
#include "../HeaderFiles/Common.h"
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
    char *message, *receivedMes;
    message = malloc(BUFSIZ + 1);
    receivedMes = malloc(BUFSIZ + 1);
    struct sockaddr_in server_addr, client_addr;
    // int client = socket(AF_INET, SOCK_STREAM, 0);
    int server = socket(AF_INET, SOCK_STREAM, 0);

    // if (setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    //     perror_exit("setsockopt(SO_REUSEADDR) failed");

    // if (client < 0)
    //     printf("Error in client creating\n");
    // else
    //     printf("Client Created\n");

    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(arguments->serverIP);
    server_addr.sin_port = htons(arguments->serverPort);

    // binding client with that port
    // client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(clientIP);
    client_addr.sin_port = htons(arguments->clientPort);

    // ----------------> esablish client socket
    printf("clientip %s, clientport %d,  serverip %s, serverport %d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    // if (bind(client, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in)) == 0)
    //     printf("Binded Correctly\n");
    // else
    //     printf("Unable to bind\n");
    // if (listen(client, 3) < 0)
    //     perror_exit("listen");
    // printf("Listening  for  connections  to port %d\n", arguments->clientPort);
    int con = connect(server, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if (con == 0)
        printf("Client Connected\n");
    else
        printf("Error in Connection\n");
    
    sendLogOn(client_addr, server);
    recv(server, receivedMes, BUFSIZ, 0);
    printf("--------------->Server : %s\n", receivedMes);
    // sendGetClients(client_addr, server);
    // ---------------------------------------------------------------------------

    //set of socket descriptors
    fd_set readfds;
    int opt = 1, max_sd;
    int client, addrlen, new_socket, client_socket[30],
        max_clients = 30, activity, i, valread, sd;
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if ((client = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if (setsockopt(client, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //bind the socket to localhost port 8888
    if (bind(client, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", arguments->clientPort);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(client, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //accept the incoming connection
    addrlen = sizeof(client_addr);
    puts("Waiting for connections ...");
    while (1)
    {
        //clear the socket set
        FD_ZERO(&readfds);
        //add master socket to set
        FD_SET(client, &readfds);
        max_sd = client;
        //add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            //highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(client, &readfds))
        {
            if ((new_socket = accept(client, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n ", new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            //send new connection greeting message
            // sprintf(message, "Welcome client with id %d\n", new_socket);
            // if (send(new_socket, message, strlen(message), 0) != strlen(message))
            // {
            //     perror("send");
            // }

            // puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }
        //else its some IO operation on some other socket
        // -------------------------------> mpalitsa
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read(sd, receivedMes, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                }
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    receivedMes[valread] = '\0';
                    // send(sd, buffer, strlen(buffer), 0);
                    if (!strcmp(receivedMes, "GET_FILE_LIST"))
                    {
                        // logOn(&headList, receivedMes, sd, max_clients, client_socket);
                    }
                    else if (strstr(receivedMes, "GET_FILE") != NULL)
                    {
                        // getClients(&headList, sd);
                    }
                    else if (!strcmp(receivedMes, "LOG_OFF"))
                    {
                        // logOff(&headList, sd, max_clients, client_socket);
                    }
                    // possible responses from Server
                    else if (strstr(receivedMes, "CLIENT_LIST") != NULL)
                    {
                        printf("I got a client list %s \n", receivedMes);

                        sleep(30);
                        sendLogOff(client_addr, client);
                    }
                    else if (!strcmp(receivedMes, "WELCOME")){
                        sendGetClients(client_addr, client);
                    }
                    else
                    {
                        // it's just a message from server
                        printf("Server told me--->: %s \n", receivedMes);

                    }
                }
            }

            // ->>>>>>>>>>>>>>>>
        }
    }

    // ===========================================================================

    

    // ------------------------ LOG_ON---------------------
    // sprintf(message, "LOG_ON < %s , %d >", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    // send(server, message, BUFSIZ, 0);
    // recv(server, receivedMes, BUFSIZ, 0);
    // printf("Server : %s\n", receivedMes);
    // // ------------------------ GET_CLIENTS----------------
    // sprintf(message, "GET_CLIENTS");
    // send(server, message, 13, 0);
    // recv(server, receivedMes, BUFSIZ, 0);
    // printf("Server : %s\n", receivedMes);

    // sleep(30);
    // // ------------------------ LOG_OFF----------------
    // sprintf(message, "LOG_OFF");
    // send(server, message, 9, 0);
    // recv(server, receivedMes, BUFSIZ, 0);
    // printf("Server : %s\n", receivedMes);
    // close(server);

    // ----------------------- Make client a server to listen to its port
    // ----------------------- already binded ip port of client

    return NULL;
}

void sendLogOn(struct sockaddr_in client_addr, int server)
{
    // ------------------------ LOG_ON---------------------
    char *message;
    message = malloc(BUFSIZ + 1);
    sprintf(message, "LOG_ON < %s , %d >", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    send(server, message, BUFSIZ, 0);
    free(message);
}

void sendGetClients(struct sockaddr_in client_addr, int server)
{
    // ------------------------ GET_CLIENTS---------------------
    char *message;
    message = malloc(14);
    sprintf(message, "GET_CLIENTS");
    send(server, message, 13, 0);
    free(message);
}

void sendLogOff(struct sockaddr_in client_addr, int server)
{
    // ------------------------ LOG_OFF---------------------
    char *message;
    message = malloc(10);
    sprintf(message, "LOG_OFF");
    send(server, message, 9, 0);
    free(message);
    close(server);
}