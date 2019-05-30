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

Buffer myBuffer;
extern pthread_cond_t cond_nonempty;
extern pthread_cond_t cond_nonfull;
char *clientIP;
int port, server;
Node *ClientsListHead;

void terminating()
{
    printf("terminating-------------->\n");
    sendLogOff(clientIP, port, server);
    pthread_exit(NULL);
    exit(0);
}

void *threadsWork(void *args)
{
    printf("--------------------> threads work \n");
    fflush(stdout);
    struct args_Workers *arguments;
    arguments = (struct args_Workers *)args;
    buffer_entry temp;
    int sock = 0;
    // struct sockaddr_in client_addr;
    char *buffer;
    char *request = malloc(BUFSIZ);
    buffer = malloc(BUFSIZ);
    while (1)
    {
        temp = retrieve();
        pthread_cond_signal(&cond_nonfull);
        // if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        // {
        //     printf("\n Socket creation error \n");
        //     pthread_exit(0);
        // }

        // memset(&client_addr, '0', sizeof(client_addr));

        // client_addr.sin_family = AF_INET;
        // client_addr.sin_addr.s_addr = inet_addr(temp.IPaddress);
        // client_addr.sin_port = htons(temp.portNum);

        // if (connect(sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        // {
        //     printf("\nConnection Failed \n");
        //     pthread_exit(0);
        // }
        sock = connect_to_socket(temp.IPaddress, temp.portNum);
        // begin doing things
        if (!strcmp(temp.pathname, "-1"))
        {
            // then it is not a file i should send a get file list
            //
            send(sock, "GET_FILE_LIST", 15, 0);
            recv(sock, buffer, 1024, 0);
            close(sock);
            printf("buffer is -> %s \n", buffer);
            // insert in buffer
            readFileList(buffer, temp.IPaddress, temp.portNum);
        }
        else
        {
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
            sprintf(fullPath, "%s/%s_%d/%s", arguments->dirName, temp.IPaddress, temp.portNum, temp.pathname);
            printf("the name of the file is %s \n", fullPath);
            if (stat(fullPath, &info) != 0)
            {
                fprintf(stderr, "stat() error on %s: %s\n", fullPath, strerror(errno));
                pthread_exit(NULL);
            }
            else if (S_ISREG(info.st_mode))
            {
                // it is a file
                // send the local version
                char *version = malloc(33);
                strcpy(version, calculateMD5hash(temp.pathname));
                sprintf(request, "GET_FILE < %s , %s >", temp.pathname, version);
                send(sock, request, strlen(request) + 1, 0);
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
                    printf("File not up to date");
                    // 001 means file exists but not up to date
                    sprintf(request, "GET_FILE < %s , 001 >", temp.pathname);
                    send(sock, request, strlen(request) + 1, 0);
                    recv(sock, buffer, BUFSIZ, 0);
                    // receive all the info to put the contents in the file
                    readFile(buffer, sock, fullPath);
                }
                close(sock);
                free(version);
            }
            else
            {
                // not exists so i have to create it
                // 000 means it doesn't exist
                sprintf(request, "GET_FILE < %s , 000 >", temp.pathname);
                send(sock, request, strlen(request) + 1, 0);
                recv(sock, buffer, BUFSIZ, 0);
                // now I have received the first part of a receive file operation
                // create the file fullPath
                char *command = malloc(BUFSIZ);
                sprintf(command, "./createFileDirs.sh %s", fullPath);
                system(command);
                free(command);
                // receive all the info to put the contents in the file
                readFile(buffer, sock, fullPath);

                close(sock);
            }

            free(fullPath);
        }
    }

    pthread_exit(0);
}


// I now need to put the new select method in here and replace the one that exists now
// https://www.gnu.org/software/libc/manual/html_node/Server-Example.html?fbclid=IwAR1PHqrG94YX21XfsHb_77PDzgNFAr4inv-COyXCo5CcJEQZRi5MkAmFFfU
// ---------------->
void *Mainthread(void *args)
{
    pthread_mutex_init(&mutexList, NULL);
    struct args_MainThread *arguments;
    ClientsListHead = NULL;
    clientIP = malloc(25);
    arguments = (struct args_MainThread *)args;
    char *receivedMes;
    receivedMes = malloc(BUFSIZ + 1);
    strcpy(receivedMes, "");
    struct sockaddr_in server_addr, client_addr;
    server = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(arguments->serverIP);
    server_addr.sin_port = htons(arguments->serverPort);

    // binding client with that port
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(arguments->myIP);
    client_addr.sin_port = htons(arguments->clientPort);
    port = arguments->clientPort;
    // signal(SIGINT, terminating);
    strcpy(clientIP, arguments->myIP);
    struct sigaction a;
    a.sa_handler = terminating;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);
    sigaction(SIGINT, &a, NULL);
    // initialize buffer
    init(arguments->bufSize);
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
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n ", new_socket, clientIP, arguments->clientPort);
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
                        printf("received get file list ... \n");
                        char *dir;
                        dir = malloc(512);
                        strcpy(dir, arguments->dirName);
                        sendFileList(dir, sd);
                        free(dir);
                    }
                    else if (strstr(receivedMes, "GET_FILE") != NULL)
                    {
                        printf("received get file ... \n");
                        char *dir;
                        char *version;
                        char *path;
                        dir = malloc(512);
                        version = malloc(33);
                        path = malloc(BUFSIZ);
                        strcpy(dir, arguments->dirName);
                        // get the filename and version
                        sscanf(receivedMes, "GET_FILE < %s , %s >", path, version);
                        sendFile(dir, path, version, sd);
                        free(dir);
                        free(path);
                        free(version);
                    }
                    else if (strstr(receivedMes, "USER_OFF") != NULL)
                    {
                        printf("received user off.. %s\n", receivedMes);
                        deleteFromList(receivedMes);
                    }
                    else if (strstr(receivedMes, "CLIENT_LIST") != NULL)
                    {
                        // tokenize it and push it in to list with mutexes
                        printf("received client list.. %s\n", receivedMes);
                        tokenizeClientList(receivedMes);
                        // now I have all of the clients in my list
                        // put the requests in the buffer for all the entries in my list
                        putRequestsInBuffer();
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
                    else if (strstr(receivedMes, "FILE_LIST") != NULL)
                    {
                    }
                    else if (strstr(receivedMes, "FILE_SIZE") != NULL)
                    {
                    }
                    else if (!strcmp(receivedMes, "FILE_UP_TO_DATE"))
                    {
                        printf("I received %s\n", receivedMes);
                    }
                    else if (!strcmp(receivedMes, "FILE_NOT_FOUND"))
                    {
                        printf("I received %s\n", receivedMes);
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
    // return NULL;
    pthread_exit(0);
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

// char *strremove(char *str, const char *sub)
// {
//     size_t len = strlen(sub);
//     if (len > 0)
//     {
//         char *p = str;
//         while ((p = strstr(p, sub)) != NULL)
//         {
//             memmove(p, p + len, strlen(p + len) + 1);
//         }
//     }
//     return str;
// }

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
        pthread_cond_signal(&cond_nonempty);
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
    pthread_cond_signal(&cond_nonempty);
    free(IP);
}

void findFiles(char *source, int indent, char **result, int *NumOfFiles)
{
    DIR *dir;
    struct dirent *entry;
    char path[1025];
    struct stat info;
    char *temp;
    temp = malloc(60);
    if ((dir = opendir(source)) == NULL)
        perror("opendir() error");
    else
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_name[0] != '.')
            {
                strcpy(path, source);
                strcat(path, "/");
                strcat(path, entry->d_name);
                if (stat(path, &info) != 0)
                {
                    fprintf(stderr, "stat() error on %s: %s\n", path, strerror(errno));
                }
                else if (S_ISDIR(info.st_mode))
                {
                    // it is a directory
                    findFiles(path, indent + 1, result, NumOfFiles);
                }
                else if (S_ISREG(info.st_mode))
                {
                    // it is a file
                    (*NumOfFiles) = (*NumOfFiles) + 1;
                    char *version, *str1, *str2;
                    str1 = malloc(BUFSIZ);
                    str2 = malloc(60);
                    version = malloc(33);
                    strcpy(version, calculateMD5hash(path));
                    sprintf(temp, "< %s , %s > ", path, version);
                    strcpy(str1, *result);
                    strcpy(str2, temp);
                    appendString(result, str1, str2);
                    free(version);
                    free(str1);
                    free(str2);
                }
            }
        }
        closedir(dir);
    }
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

    printf("final result-----------> %s \n", result);
    // so now just send this final result to the other
    send(clientSocket, result, strlen(result), 0);
    close(clientSocket);
    free(str1);
    free(str2);
    free(temp);
    free(result);
}

void sendFile(char *dirName, char *pathName, char *version, int socketSD)
{
    char *versionNow, *fullPath;
    versionNow = malloc(33);
    fullPath = malloc(BUFSIZ);
    struct stat info;
    appendString(&fullPath, dirName, pathName);
    if (stat(fullPath, &info) != 0)
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
    strcpy(versionNow, calculateMD5hash(fullPath));
    if (!strcmp(versionNow, version))
    {
        // same version
        send(socketSD, "FILE_UP_TO_DATE", 17, 0);
        return;
    }
    // different one
    // so send all of its contents in chuncks of BUFSIZ
    sendFileContents(fullPath, socketSD, versionNow);
}

long long countSize(char *filename)
{
    struct stat sb;
    if (stat(filename, &sb) == -1)
    {
        perror("stat");
        exit(1);
    }

    printf("File size: %lld bytes\n", (long long)sb.st_size);
    return ((long long)sb.st_size);
}

void sendFileContents(char *pathName, int socketSD, char *version)
{
    FILE *fp;
    long long size;
    size_t bytesRead = 0;
    char *result;
    result = malloc(BUFSIZ + 1);
    strcpy(result, "");
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
    sprintf(result, "FILE_SIZE %s %lld ", version, size);
    send(socketSD, result, strlen(result), 0);

    // read BUFSIZ and send and then again till it is done
    strcpy(result, "");
    bytesRead = fread(result, BUFSIZ, 1, fp);
    // read the first chunk
    while (bytesRead > 0)
    {
        send(socketSD, result, strlen(result), 0);

        strcpy(result, "");
        bytesRead = fread(result, BUFSIZ, 1, fp);
    }

    close(socketSD);
    fclose(fp);
}

char *calculateMD5hash(char *pathname)
{
    char *result;
    char *fileWithHash;
    char *temp;
    char *command;
    FILE *fp;
    result = malloc(33);
    temp = malloc(strlen(pathname) + 1);
    fileWithHash = malloc(strlen(pathname) + 4);
    strcpy(result, "");
    // call the script to make the file with the hash then read it from there
    sprintf(fileWithHash, "%s.md5", pathname);

    command = malloc(1025);
    sprintf(command, "md5sum %s > %s", pathname, fileWithHash);
    system(command);
    free(command);

    // now that the file is ready with the checksum
    // open the file and extract the hash

    fp = fopen(fileWithHash, "r");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "%s %s", result, temp);

    fclose(fp);
    remove(fileWithHash);
    free(temp);
    free(fileWithHash);
    return result;
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

        put(temp);
        pthread_cond_signal(&cond_nonempty);
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
    char *chunk;
    char *version;
    int bytes = 0;
    int times = 0;
    sourceStr = malloc(strlen(source) + 1);
    tobeRemov = malloc(50);
    version = malloc(33);
    chunk = malloc(BUFSIZ + 1);
    strcpy(sourceStr, source);
    sscanf(sourceStr, "FILE_SIZE %s %d ", version, &bytes);
    times = bytes / BUFSIZ + 1;
    fp = fopen(fullPath, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "error in opening file readfile\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < times; i++)
    {
        strcpy(chunk, "");
        recv(socketSD, chunk, BUFSIZ, 0);
        // so now I have part of the file's
        fwrite(chunk, 1, sizeof(chunk), fp);
    }

    fclose(fp);
    close(socketSD);
    free(sourceStr);
    free(tobeRemov);
    free(version);
    free(chunk);
}