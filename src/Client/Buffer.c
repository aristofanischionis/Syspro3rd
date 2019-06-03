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

#include "headerfile.h"
#include "../HeaderFiles/Common.h"
int BUF_SIZE;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pthread_mutex_t mutexBuffer;
extern Buffer myBuffer;

// this is the same as the example from mr Ntoulas slides
void init(int bufSize)
{
    BUF_SIZE = bufSize;
    pthread_cond_init(&cond_nonempty, NULL);
    pthread_cond_init(&cond_nonfull, NULL);
    pthread_mutex_init(&mutexBuffer, NULL);
    myBuffer.elements = malloc(bufSize * sizeof(struct buffer_entry));
    myBuffer.start = 0;
    myBuffer.end = -1;
    myBuffer.count = 0;
}

void put(buffer_entry data)
{
    pthread_mutex_lock(&mutexBuffer);
    while (myBuffer.count >= BUF_SIZE)
    {
        pthread_cond_wait(&cond_nonfull, &mutexBuffer);
    }
    myBuffer.end = (myBuffer.end + 1) % BUF_SIZE;
    myBuffer.elements[myBuffer.end] = data;
    myBuffer.count++;
    pthread_mutex_unlock(&mutexBuffer);
    pthread_cond_signal(&cond_nonempty);
}

struct buffer_entry retrieve()
{
    buffer_entry data;
    strcpy(data.IPaddress, "");
    strcpy(data.pathname, "");
    strcpy(data.version, "-1");
    data.portNum = 0;
    pthread_mutex_lock(&mutexBuffer);
    while (myBuffer.count <= 0)
    {
        pthread_cond_wait(&cond_nonempty, &mutexBuffer);
    }
    data = myBuffer.elements[myBuffer.start];
    myBuffer.start = (myBuffer.start + 1) % BUF_SIZE;
    myBuffer.count--;
    pthread_mutex_unlock(&mutexBuffer);
    pthread_cond_signal(&cond_nonfull);
    return data;
}

void destroy()
{
    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&mutexBuffer);
    free(myBuffer.elements);
}

