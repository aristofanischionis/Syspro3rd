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
#include <limits.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/time.h>
#include "headerfile.h"
#include "../HeaderFiles/Common.h"
#include "../HeaderFiles/LinkedList.h"

pthread_mutex_t mutexList;

Buffer myBuffer;
extern pthread_cond_t cond_nonempty;
extern pthread_cond_t cond_nonfull;
pthread_mutex_t mutex;

pthread_cond_t cond_wakeup;
char *clientIP;
// int port, server;
Node *ClientsListHead;
#define MAX_FILE_SIZE 3000

void terminating()
{
    printf("terminating-------------->\n");
    // sendLogOff(clientIP, port, server);
    pthread_exit(NULL);
    exit(0);
}

void *threadsWork(void *args)
{
    // printf("--------------------> threads work \n");
    // fflush(stdout);
    struct args_Workers *arguments;
    arguments = (struct args_Workers *)args;
    buffer_entry temp;
    int sock = 0;
    // struct sockaddr_in client_addr;
    char *buffer;
    char *request = malloc(BUFSIZ);
    buffer = malloc(2*BUFSIZ);
    pthread_cond_wait(&cond_wakeup, &mutex);
    while (1)
    {
        // printf("threads in while loopa \n");
        temp = retrieve();
        // pthread_cond_signal(&cond_nonfull);
        
        sock = connect_to_socket(temp.IPaddress, temp.portNum);
        // begin doing things
        if (!strcmp(temp.pathname, "-1"))
        {
            // then it is not a file i should send a get file list
            //
            printf("Sending get file list \n");
            send(sock, "GET_FILE_LIST", 15, 0);
            strcpy(buffer, "");
            recv(sock, buffer, 1024, 0);
            // close(sock);
            printf("buffer is -> %s \n", buffer);
            // insert in buffer
            readFileList(buffer, temp.IPaddress, temp.portNum);
        }
        else
        {
            printf("Sending get file \n");
            pthread_mutex_lock(&mutexList);
            // it is a file
            if (exists(&ClientsListHead, temp.IPaddress, temp.portNum) == 0)
            {
                perror("--> client not in list");
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&mutexList);
            char *fullPath = malloc(BUFSIZ);
            struct stat info;
            sprintf(fullPath, "%s_%d/%s_%d/%s", arguments->myIP, arguments->myPort, temp.IPaddress, temp.portNum, temp.pathname);
            // printf("the name of the file is %s \n", fullPath);
            if (stat(fullPath, &info) != 0)
            {
                // fprintf(stderr, "stat() error on %s: %s\n", fullPath, strerror(errno));
                // pthread_exit(NULL);
                printf("File doesn't exist! \n");
                // not exists so i have to create it
                // 000 means it doesn't exist
                sprintf(request, "GET_FILE < %s , 000 >", temp.pathname);
                send(sock, request, strlen(request) + 1, 0);
                strcpy(buffer, "");
                recv(sock, buffer, BUFSIZ, 0);
                // now I have received the first part of a receive file operation
                // create the file fullPath
                char *command = malloc(BUFSIZ+1);
                char buf[BUFSIZ + 1];
                char *res = realpath("./src/Client/createFileDirs.sh", buf);
                if (!res) {
                    // printf("This source is at %s.\n", buf);
                    perror("realpath");
                    pthread_exit(NULL);
                }
                
                sprintf(command, "%s %s", buf,fullPath);
                system(command);
                free(command);
                // receive all the info to put the contents in the file
                readFile(buffer, sock, fullPath);

                // close(sock);
            }
            else if (S_ISREG(info.st_mode))
            {
                // it is a file
                // send the local version
                printf("file exists in backup\n");
                char *version = malloc(33);
                // this is the fiile starting with 127. ....
                char* backupPath = malloc(1025);

                sprintf(backupPath, "%s_%d/%s_%d/%s",arguments->myIP, arguments->myPort, temp.IPaddress, temp.portNum, temp.pathname);
                strcpy(version, calculateMD5hash(backupPath));
                strcpy(temp.version, version);
                free(backupPath);
                sprintf(request, "GET_FILE < %s , %s >", temp.pathname, version);
                send(sock, request, strlen(request) + 1, 0);
                strcpy(buffer, "");
                recv(sock, buffer, BUFSIZ, 0);
                // now I have the response in buffer
                if (!strcmp(buffer, "FILE_UP_TO_DATE"))
                {
                    //file up to date don't do anything
                    printf("File is up to date \n");
                }
                else
                {
                    // not up to date
                    printf("File not up to date\n");
                    // 001 means file exists but not up to date
                    sprintf(request, "GET_FILE < %s , 001 >", temp.pathname);
                    send(sock, request, strlen(request) + 1, 0);
                    recv(sock, buffer, BUFSIZ, 0);
                    // receive all the info to put the contents in the file
                    readFile(buffer, sock, fullPath);
                }
                // close(sock);
                free(version);
            }
            else
            {
                // not exists so i have to create it
                // 000 means it doesn't exist
                printf("weird things happen \n");
                pthread_exit(NULL);
            }
            free(fullPath);
        }
        close(sock);
        sock = -1;
    }
    pthread_exit(0);
}

void *Mainthread(void *args)
{
    fd_set active_fd_set, read_fd_set;
    int i;
    int server;
    struct sockaddr_in clientname;
    size_t size;
    struct args_MainThread *arguments;
    pthread_mutex_init(&mutexList, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_wakeup, NULL);
    ClientsListHead = NULL;
    clientIP = malloc(25);
    arguments = (struct args_MainThread *)args;

    server = connect_to_socket(arguments->serverIP, arguments->serverPort);

    // port = arguments->clientPort;
    // signal(SIGINT, terminating);
    strcpy(clientIP, arguments->myIP);
    // struct sigaction a;
    // a.sa_handler = terminating;
    // a.sa_flags = 0;
    // sigemptyset(&a.sa_mask);
    // sigaction(SIGINT, &a, NULL);
    // initialize buffer
    init(arguments->bufSize);
    // ----------------> establish client socket
    printf("clientip %s, clientport %d,  serverip %s, serverport %d \n", arguments->myIP, arguments->clientPort, arguments->serverIP, arguments->serverPort);

    int client = make_socket(arguments->myIP, arguments->clientPort);
    // make a new socket

    if (listen(client, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* Initialize the set of active sockets. */
    FD_ZERO(&active_fd_set);
    FD_SET(client, &active_fd_set);
    // send the log on
    sendLogOn(arguments->myIP, arguments->clientPort, server);
    while (1)
    {
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i < FD_SETSIZE; ++i)
            if (FD_ISSET(i, &read_fd_set))
            {
                if (i == client)
                {
                    /* Connection request on original socket. */
                    int new;
                    size = sizeof(clientname);
                    new = accept(client,
                                 (struct sockaddr *)&clientname,
                                 &size);
                    if (new < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stderr,
                            "Client: connect from host %s, port %d.\n",
                            inet_ntoa(clientname.sin_addr),
                            ntohs(clientname.sin_port));
                    FD_SET(new, &active_fd_set);
                }
                else
                {
                    /* Data arriving on an already-connected socket. */
                    char *dir;
                    dir = malloc(512);
                    strcpy(dir, arguments->dirName);
                    if (read_from_client1(i, dir) < 0)
                    {
                        close(i);
                        FD_CLR(i, &active_fd_set);
                    }
                }
            }
    }
}

int read_from_client1(int socketD, char *dir)
{
    char buffer[BUFSIZ + 1];
    int nbytes;

    nbytes = read(socketD, buffer, BUFSIZ);
    if (nbytes < 0)
    {
        /* Read error. */
        perror("read");
        exit(EXIT_FAILURE);
    }
    else if (nbytes == 0)
        /* End-of-file. */
        return -1;
    else
    {
        /* Data read. */
        fprintf(stderr, "Client: got message: '%s'\n", buffer);
        if (!strcmp(buffer, "GET_FILE_LIST"))
        {
            sendFileList(dir, socketD);
        }
        else if (strstr(buffer, "GET_FILE") != NULL)
        {
            char *version;
            char *path;
            version = malloc(33);
            path = malloc(BUFSIZ);
            // get the filename and version
            sscanf(buffer, "GET_FILE < %s , %s >", path, version);
            sendFile(path, version, socketD);
            free(path);
            free(version);
        }
        else if (strstr(buffer, "USER_OFF") != NULL)
        {
            printf("received user off.. %s\n", buffer);
            deleteFromList(buffer);
        }
        else if (strstr(buffer, "CLIENT_LIST") != NULL)
        {
            // tokenize it and push it in to list with mutexes
            tokenizeClientList(buffer);
            // now I have all of the clients in my list
            // put the requests in the buffer for all the entries in my list
            putRequestsInBuffer();
            //wake up threads
            pthread_cond_broadcast(&cond_wakeup);
        }
        else if (!strcmp(buffer, "WELCOME"))
        {
            sendGetClients(socketD);
        }
        else if (strstr(buffer, "USER_ON") != NULL)
        {
            insertInClientList(buffer);
        }
        else
        {
            // it's just a message from server
            printf("Server told me--->: %s \n", buffer);
        }
        return 0;
    }
}

void sendLogOn(char *myIP, int myPort, int server)
{
    // ------------------------ LOG_ON---------------------
    char *message;
    message = malloc(BUFSIZ + 1);
    sprintf(message, "LOG_ON < %s , %d >", myIP, myPort);
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
    // close(server);
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
        free(clientListStr);
        free(tobeRemov);
        free(IP);
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

void putRequestsInBuffer()
{
    // I will extract info from list and place it in buffer entries to put it in buffer
    buffer_entry temp;
    Node *node = ClientsListHead;
    pthread_mutex_lock(&mutexList);

    while (node != NULL)
    {
        strcpy(temp.IPaddress, node->IP);
        temp.portNum = node->port;
        strcpy(temp.pathname, "-1");
        strcpy(temp.version, "-1");
        // temp is the ready request so put it in buffer
        put(temp);
        // pthread_cond_signal(&cond_nonempty);
        node = node->next;
    }
    pthread_mutex_unlock(&mutexList);
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
    buffer_entry temp;
    char *IP;
    int port = 0;
    IP = malloc(20);

    sscanf(input, "USER_ON < %s , %d >", IP, &port);

    pthread_mutex_lock(&mutexList);
    push(&ClientsListHead, IP, port);
    pthread_mutex_unlock(&mutexList);
    // put the request in the circular buffer as well
    strcpy(temp.IPaddress, IP);
    temp.portNum = port;
    strcpy(temp.pathname, "-1");
    strcpy(temp.version, "-1");
    // temp is the ready request so put it in buffer
    put(temp);
    // pthread_cond_signal(&cond_nonempty);
    free(IP);
}

void sendFileList(char *dirName, int clientSocket)
{
    int numOfFiles = 0;
    char *result, *temp;
    char *str1, *str2;
    str1 = malloc(BUFSIZ);
    str2 = malloc(BUFSIZ);
    result = malloc(BUFSIZ);
    temp = malloc(BUFSIZ);
    findFiles(dirName, 0, &temp, &numOfFiles);
    // now I should have the num of files and the string with all the files in temp
    sprintf(str1, "FILE_LIST %d ", numOfFiles);
    strcpy(str2, temp);
    appendString(&result, str1, str2);

    // printf("my filelist to send is-----------> %s \n", result);
    // so now just send this final result to the other
    send(clientSocket, result, strlen(result), 0);
    // close(clientSocket);
    free(str1);
    free(str2);
    free(temp);
    free(result);
}

void sendFile(char *pathName, char *version, int socketSD)
{
    char *versionNow;
    versionNow = malloc(33);
    struct stat info;
    if (stat(pathName, &info) != 0)
    {
        // there is not a file
        send(socketSD, "FILE_NOT_FOUND", 16, 0);
        return;
    }
    else if (!S_ISREG(info.st_mode))
    {
        fprintf(stderr, "Unknown case\n");
        return;
    }
    // there is a file
    strcpy(versionNow, calculateMD5hash(pathName));
    if (!strcmp(versionNow, version))
    {
        // same version
        send(socketSD, "FILE_UP_TO_DATE", 17, 0);
        return;
    }
    // different one
    // so send all of its contents in chuncks of BUFSIZ
    sendFileContents(pathName, socketSD, versionNow);
}


void sendFileContents(char *pathName, int socketSD, char *version)
{
    FILE *fp;
    long long size;
    char* result;
    result = malloc(MAX_FILE_SIZE + 50);
    // open file
    fp = fopen(pathName, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "error in getFileContents\n");
        exit(0);
    }
    // count size
    size = countSize(pathName);
    //
    char *toBeRead;
    toBeRead = malloc(size + 1);
    strcpy(toBeRead, "");
    fread(toBeRead, (int)size, 1, fp);
    fclose(fp);
    
    sprintf(result, "FILE_SIZE %s %d %s", version, (int)size, toBeRead);
    // printf("--> %s \n", result);
    send(socketSD, result, strlen(result)+1, 0);

    free(result);
    free(toBeRead);
}


void readFileList(char *source, char *IPsender, int portSender)
{
    char *sourceStr;
    char *tobeRemov;
    char *pathName;
    char *version;
    int numOfFiles = 0;
    sourceStr = malloc(strlen(source) + 1);
    tobeRemov = malloc(50);
    pathName = malloc(BUFSIZ);
    version = malloc(33);
    strcpy(sourceStr, source);
    sscanf(sourceStr, "FILE_LIST %d ", &numOfFiles);

    if (numOfFiles == 0)
    {
        printf("This client has no files \n");
        return;
    }
    sprintf(tobeRemov, "FILE_LIST %d ", numOfFiles);
    sourceStr = strremove(sourceStr, tobeRemov);
    for (int i = 0; i < numOfFiles; i++)
    {
        buffer_entry temp;
        sscanf(sourceStr, "< %s , %s > ", pathName, version);
        // now I have extracted the required info
        // just place it in the buffer
        strcpy(temp.IPaddress, IPsender);
        strcpy(temp.pathname, pathName);
        strcpy(temp.version, version);
        temp.portNum = portSender;
        printf("-------------> %d %s \n", temp.portNum, temp.pathname);
        put(temp);
        // pthread_cond_signal(&cond_nonempty);
        sprintf(tobeRemov, "< %s , %s > ", pathName, version);
        sourceStr = strremove(sourceStr, tobeRemov);
    }

    free(sourceStr);
    free(tobeRemov);
    free(version);
    free(pathName);
}

void readFile(char *source, int socketSD, char *fullPath)
{
    FILE *fp;
    char *sourceStr;
    char *tobeRemov;
    // char *chunk;
    char *version;
    char *data;
    data = malloc(MAX_FILE_SIZE);
    int bytes = 0;
    // int times = 0;
    sourceStr = malloc(strlen(source) + 1);
    tobeRemov = malloc(50);
    version = malloc(33);
    // chunk = malloc(BUFSIZ + 1);
    strcpy(sourceStr, source);
    sscanf(sourceStr, "FILE_SIZE %s %d", version, &bytes);
    // times = bytes / BUFSIZ + 1;
    fp = fopen(fullPath, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "error in opening file readfile\n");
        exit(EXIT_FAILURE);
    }
    // take all the info from sourcestr
    sprintf(tobeRemov, "FILE_SIZE %s %d ",  version, bytes);
    sourceStr = strremove(sourceStr, tobeRemov);
    // now sourcestr has only the data
    strcpy(data, sourceStr);

    // printf("----------->I will write %s to %s \n", data, fullPath);
    fwrite(data, 1, strlen(data), fp);
    

    fclose(fp);
    // close(socketSD);
    free(sourceStr);
    // free(tobeRemov);
    free(version);
    free(data);
    // free(chunk);
}