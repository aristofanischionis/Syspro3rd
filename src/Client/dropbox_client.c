#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <arpa/inet.h> 
#include "headerfile.h"
#include "../HeaderFiles/LinkedList.h"
#include "../HeaderFiles/Common.h"

#define MAX_PATH 200
pthread_t *threads;
struct args_Workers argumentsWorkers;

char* serverIPa;
int threadsNum;
int serverPorta;

void terminating()
{
    int server = connect_to_socket(serverIPa, serverPorta);
    printf("terminating this client!\n");
    sendLogOff(server);
    close(server);
    destroy();
    // // close all threads

    for(int i = threadsNum-1;i >= 0;i--){
        pthread_kill(threads[i], SIGTERM);
    }
    for(int i = threadsNum-1;i >= 0;i--){
        pthread_join(threads[i], NULL);
    }
    pthread_exit(NULL);
}


// parse command line args
void paramChecker(int n, char *argv[], char *toCheck, char **result)
{
    int i = 1;
    while (i < n)
    {
        if (strcmp(argv[i], toCheck) == 0)
        {
            if (i < n - 1)
            {
                if (argv[i + 1][0] == '-')
                {
                    printf("After %s flag a - was read\n", toCheck);
                    exit(1);
                }
                strcpy(*result, argv[i + 1]);
                return;
            }
            else
            {
                printf("Param after %s flag was not given\n", toCheck);
                printf("exiting...\n");
            }
        }
        i++;
    }
}


int main(int argc, char *argv[])
{
    struct args_MainThread arguments;
    int n = argc;
    char *dirName, *portNum, *workerThreads, *bufferSize, *serverPortStr, *serverIP;
    int port = 0, bSize = 0, serverPort = 0;
    // init strings
    dirName = (char *)malloc(MAX_PATH);
    portNum = (char *)malloc(8);
    workerThreads = (char *)malloc(5);
    bufferSize = (char *)malloc(7);
    serverPortStr = (char *)malloc(8);
    serverIP = (char *)malloc(17);
    // read all cmd line arguments
    paramChecker(n, argv, "-d", &dirName);
    paramChecker(n, argv, "-p", &portNum);
    paramChecker(n, argv, "-w", &workerThreads);
    paramChecker(n, argv, "-b", &bufferSize);
    paramChecker(n, argv, "-sp", &serverPortStr);
    paramChecker(n, argv, "-sip", &serverIP);
    // make the appropriate convertions
    port = atoi(portNum);
    threadsNum = atoi(workerThreads);
    bSize = atoi(bufferSize);
    serverPort = atoi(serverPortStr);
    // freeing up useless memory
    free(portNum);
    free(workerThreads);
    free(bufferSize);
    free(serverPortStr);

    //
    char hostbuffer[256]; 
    char *IPbuffer; 
    struct hostent *host_entry; 
    int hostname; 
  
    // To retrieve hostname 
    hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
    checkHostName(hostname); 
  
    // To retrieve host information 
    host_entry = gethostbyname(hostbuffer); 
    checkHostEntry(host_entry); 
  
    // To convert an Internet network 
    // address into ASCII string 
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    printf("Client listening @IP: %s , @port: %d\n",IPbuffer, port);
    // start
    arguments.bufSize = bSize;
    arguments.clientPort = port;
    arguments.serverPort = serverPort;
    strcpy(arguments.dirName, dirName);
    strcpy(arguments.serverIP, serverIP);
    strcpy(arguments.myIP, IPbuffer);

    ///
    strcpy(argumentsWorkers.dirName, dirName);
    strcpy(argumentsWorkers.myIP, IPbuffer);
    argumentsWorkers.myPort = port;

    // signal handler
    serverIPa = malloc(25);
    strcpy(serverIPa, serverIP);
    serverPorta = serverPort;
    signal(SIGINT, terminating);
    
    Mainthread(&arguments);

    exit(0);
}

