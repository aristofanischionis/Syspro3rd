#ifndef SERVER_HEADER
#define SERVER_HEADER
#include "../HeaderFiles/LinkedList.h"

void logOn(Node **headList, char *buffer, int sd, int max_clients, int client_socket[]);
void getClients(Node **headList, int sd);
void logOff(Node **headList, int sd, int max_clients, int client_socket[]);
#endif