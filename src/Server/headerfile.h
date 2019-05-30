#ifndef SERVER_HEADER
#define SERVER_HEADER
#include "../HeaderFiles/LinkedList.h"

int logOn(Node **headList, char *buffer, char** IP, int * port);
void getClients(Node **headList, int sd, char* IP, int port);
void logOff(Node **headList, char *buffer, int sd);
#endif