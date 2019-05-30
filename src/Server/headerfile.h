#ifndef SERVER_HEADER
#define SERVER_HEADER
#include "../HeaderFiles/LinkedList.h"

int logOn(Node **headList, char *buffer, int sd, int max_clients, int **client_socket, int *max_sd, fd_set readfds, char** IP, int * port);
void getClients(Node **headList, int sd, char* IP, int port);
void logOff(Node **headList, char* buffer, int sd, int max_clients, int client_socket[]);
void sendToAllInList(Node *headList, char* specialMessage, int specialFd, char* otherMessage);
#endif