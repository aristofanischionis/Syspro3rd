
#ifndef CLIENT_HEADER
#define CLIENT_HEADER

int Initialisation(char *serverIP, int serverPort);
void *Mainthread(void *args);

struct args_MainThread
{
    char serverIP[25];
    int serverPort;
    int clientPort;
};

#endif