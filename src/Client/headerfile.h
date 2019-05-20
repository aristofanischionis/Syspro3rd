
#ifndef CLIENT_HEADER
#define CLIENT_HEADER

int Initialisation(char *serverIP, int serverPort);
void *Mainthread(void *args);
void sendLogOn(struct sockaddr_in client_addr, int server);
void sendGetClients(struct sockaddr_in client_addr, int server);
void sendLogOff(struct sockaddr_in client_addr, int server);

struct args_MainThread
{
    char serverIP[25];
    int serverPort;
    int clientPort;
};


#endif