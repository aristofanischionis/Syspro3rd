#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>

#include "headerfile.h"
#include "../HeaderFiles/Common.h"
#include "../HeaderFiles/LinkedList.h"

pthread_mutex_t mutexList;

buffer_entry *Buffer;
char *clientIP;
int port, server;
Node *ClientsListHead;

/*  Print  error  message  and  exit  */
void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void terminating()
{
    printf("terminating-------------->\n");
    sendLogOff(clientIP, port, server);
    pthread_exit(NULL);
    exit(0);
}

void *threadsWork(void *args)
{
    struct args_Workers *arguments;
    arguments = (struct args_MainThread *)args;
}

void *Mainthread(void *args)
{
    // Two buffer are for message communication
    pthread_mutex_init(&mutexList, NULL);
    struct args_MainThread *arguments;
    ClientsListHead = NULL;
    // char *clientIP;
    clientIP = malloc(20);
    strcpy(clientIP, "127.0.0.1");
    arguments = (struct args_MainThread *)args;
    char *message, *receivedMes;
    message = malloc(BUFSIZ + 1);
    receivedMes = malloc(BUFSIZ + 1);
    strcpy(receivedMes, "");
    struct sockaddr_in server_addr, client_addr;
    server = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(arguments->serverIP);
    server_addr.sin_port = htons(arguments->serverPort);

    // binding client with that port
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(clientIP);
    client_addr.sin_port = htons(arguments->clientPort);
    port = arguments->clientPort;
    // signal(SIGINT, terminating);
    struct sigaction a;
    a.sa_handler = terminating;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);
    sigaction(SIGINT, &a, NULL);
    // ----------------> establish client socket
    printf("clientip %s, clientport %d,  serverip %s, serverport %d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    int con = connect(server, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if (con == 0)
        printf("Client Connected\n");
    else
        printf("Error in Connection\n");

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
    sendLogOn(client_addr, server);
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
                    // printf("Host disconnected , ip %s , port %d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    // //Close the socket and mark as 0 in list for reuse
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
                    }
                    else if (strstr(receivedMes, "GET_FILE") != NULL)
                    {
                    }
                    else if (strstr(receivedMes, "USER_OFF") != NULL)
                    {
                        printf("received user off.. %s\n", receivedMes);
                        deleteFromList(receivedMes);
                    }
                    // possible responses from Server
                    else if (strstr(receivedMes, "CLIENT_LIST") != NULL)
                    {
                        // tokenize it and push it in to list with mutexes
                        printf("received client list.. %s\n", receivedMes);
                        tokenizeClientList(receivedMes);
                    }
                    else if (!strcmp(receivedMes, "WELCOME"))
                    {
                        printf("sending get clients..\n");
                        sendGetClients(server);
                    }
                    else if (strstr(receivedMes, "USER_ON") != NULL)
                    {
                        printf("I received info, user on %s\n", receivedMes);
                        insertInClientList(receivedMes);
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

    // pthread_cond_destroy(&cond);
    // pthread_mutex_destroy(&mutex);
    // ===========================================================================
    return NULL;
}

void sendLogOn(struct sockaddr_in client_addr, int server)
{
    // ------------------------ LOG_ON---------------------
    char *message;
    message = malloc(BUFSIZ + 1);
    sprintf(message, "LOG_ON < %s , %d >", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    send(server, message, strlen(message), 0);
    free(message);
}

void sendGetClients(int server)
{
    // ------------------------ GET_CLIENTS---------------------
    char *message;
    message = malloc(14);
    sprintf(message, "GET_CLIENTS");
    send(server, message, 13, 0);
    free(message);
}

void sendLogOff(char *IP, int port, int server)
{
    // ------------------------ LOG_OFF---------------------
    char *message;
    message = malloc(BUFSIZ + 1);
    sprintf(message, "LOG_OFF < %s , %d >", IP, port);
    printf("eimai o client-------> message: %s \n", message);
    send(server, message, strlen(message), 0);
    free(message);
    close(server);
}

char *strremove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if (len > 0)
    {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL)
        {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

// input str should be like this:
// "CLIENT_LIST 3 < 123.23.2.2 , 20 > < 113.13.1.1 , 10 > < 111.23.2.2 , 15 > "
void tokenizeClientList(char *input)
{
    char *clientListStr;
    char *IP;
    char *tobeRemov;
    int port = 0;
    int clientsNum = 0;
    clientListStr = malloc(strlen(input) + 1);
    tobeRemov = malloc(50);
    IP = malloc(20);
    strcpy(clientListStr, input);
    sscanf(clientListStr, "CLIENT_LIST %d", &clientsNum);
    if (clientsNum == 0)
    {
        printf("I am the only client \n");
        return;
    }
    sprintf(tobeRemov, "CLIENT_LIST %d ", clientsNum);
    clientListStr = strremove(clientListStr, tobeRemov);
    for (int i = 0; i < clientsNum; i++)
    {
        // ClientsListHead
        sscanf(clientListStr, "< %s , %d > ", IP, &port);
        // printf("-----> Ip |%s| port %d \n", IP, port);
        // push it in the client list
        // but firstly lock mutex
        pthread_mutex_lock(&mutexList);
        push(&ClientsListHead, IP, port);
        pthread_mutex_unlock(&mutexList);
        sprintf(tobeRemov, "< %s , %d > ", IP, port);
        clientListStr = strremove(clientListStr, tobeRemov);
    }

    free(clientListStr);
    free(tobeRemov);
    free(IP);
}

void deleteFromList(char *input)
{
    char *IP;
    int port = 0;
    IP = malloc(20);

    sscanf(input, "USER_OFF < %s , %d >", IP, &port);

    pthread_mutex_lock(&mutexList);
    deleteNode(&ClientsListHead, IP, port);
    pthread_mutex_unlock(&mutexList);

    free(IP);
}

void insertInClientList(char *input)
{
    char *IP;
    int port = 0;
    IP = malloc(20);

    sscanf(input, "USER_ON < %s , %d >", IP, &port);

    pthread_mutex_lock(&mutexList);
    push(&ClientsListHead, IP, port);
    pthread_mutex_unlock(&mutexList);

    free(IP);
}

void sendFileList(char *dirName)
{
}

void sendFile()
{
}

char *calculateMD5hash(char *pathname)
{
    char *result;
    char *fileWithHash;
    char *temp;
    pid_t pid;
    FILE *fp;
    result = malloc(33);
    temp = malloc(strlen(pathname) + 1);
    fileWithHash = malloc(strlen(pathname) + 3);
    strcpy(result, "");
    // call the script to make the file with the hash then read it from there
    pid = fork();
    if (pid == -1)
    {
        perror("fork in calculate md5 hash: ");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        char *argvList[] = {"./MD5hash.sh", pathname, NULL};
        execv("./MD5hash.sh", argvList);
        exit(EXIT_FAILURE);
    }
    wait(NULL);
    // now that the file is ready with the checksum
    // open the file and extract the hash

    sprintf(fileWithHash, "%s.md", pathname);

    fp = fopen(fileWithHash, "r");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "MD5 (%s) = %s", temp, result);

    fclose(fp);
    remove(fileWithHash);
    free(temp);
    free(fileWithHash);
    return result;
}