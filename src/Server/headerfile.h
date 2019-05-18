#ifndef SERVER_HEADER
#define SERVER_HEADER
#include "../HeaderFiles/LinkedList.h"

void logOn(Node *headList, char *buffer, int sd);
void getClients();
void logOff();
#endif