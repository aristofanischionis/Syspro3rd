#ifndef SERVER_HEADER
#define SERVER_HEADER
#include "../HeaderFiles/LinkedList.h"

void logOn(Node **headList, char *buffer);
void getClients(Node **headList, int sd, char* IP, int port);
void logOff(Node **headList, char *buffer, int sd);
#endif