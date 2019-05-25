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
#include "headerfile.h"
#include "../HeaderFiles/LinkedList.h"
#define MAX_PATH 200

// pthread_cond_t condBuffer;
// pthread_mutex_t mutexBuffer;

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
    struct args_Workers argumentsWorkers;
    int n = argc;
    char *dirName, *portNum, *workerThreads, *bufferSize, *serverPortStr, *serverIP;
    int port = 0, threadsNum = 0, bSize = 0, serverPort = 0;
    pthread_t *threads;
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
    threadsNum++;
    bSize = atoi(bufferSize);
    serverPort = atoi(serverPortStr);
    // freeing up useless memory
    free(portNum);
    free(workerThreads);
    free(bufferSize);
    free(serverPortStr);
    //
    // pthread_cond_init(&condBuffer, NULL);
    // pthread_mutex_init(&mutexBuffer, NULL);
    // pthread_mutex_init(&mutexList, NULL);
    //
    // Buffer = malloc(bSize * sizeof(buffer_entry));

    // start
    threads = malloc(threadsNum * sizeof(pthread_t));
    arguments.clientPort = port;
    arguments.serverPort = serverPort;

    strcpy(arguments.serverIP, serverIP);
    printf("before thread\n");
    pthread_create(&threads[0], NULL, Mainthread, &arguments);

    //

    for(int i = 1 ; i<threadsNum ;i ++){
        pthread_create(&threads[i], NULL, threadsWork, &argumentsWorkers);

    }
    //
    for(int i = 0 ; i<threadsNum ;i ++){
        pthread_join(threads[i], NULL);

    }
    // pthread_join(threads[0], NULL);

    printf("After Thread\n");
    exit(0);
}

