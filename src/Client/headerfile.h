
#ifndef CLIENT_HEADER
#define CLIENT_HEADER

int Initialisation(char *serverIP, int serverPort);
void *threadsWork(void *args);
void *Mainthread(void *args);
void sendLogOn(struct sockaddr_in client_addr, int server);
void sendGetClients(int server);
void sendLogOff(char *IP, int port, int server);
void terminating();

struct args_MainThread
{
    char serverIP[25];
    int serverPort;
    int clientPort;
};

struct args_Workers
{
};

typedef struct buffer_entry
{
    char pathname[128];
    char version[64];
    char IPaddress[20];
    int portNum;
} buffer_entry;

typedef struct buffer
{
    buffer_entry *elements;
    int start;
    int end;
    int count;
} Buffer;

struct buffer_entry retrieve();
void put(buffer_entry data);
void init(int bufSize);
void destroy();

#endif