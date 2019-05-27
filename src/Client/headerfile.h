
#ifndef CLIENT_HEADER
#define CLIENT_HEADER

int Initialisation(char *serverIP, int serverPort);
void *threadsWork(void *args);
void *Mainthread(void *args);
void sendLogOn(struct sockaddr_in client_addr, int server);
void sendGetClients(int server);
void sendLogOff(char *IP, int port, int server);
void tokenizeClientList(char *input);
char *strremove(char *str, const char *sub);
void deleteFromList(char *input);
void insertInClientList(char *input);
char *calculateMD5hash(char *pathname);
void findFiles(char *source, int indent, char **result, int *NumOfFiles);
void sendFileList(char *dirName);
void terminating();

struct args_MainThread
{
    char serverIP[25];
    int serverPort;
    int clientPort;
    char dirName[512];
};

struct args_Workers
{
};

typedef struct buffer_entry
{
    char pathname[128];
    char version[33]; // 32 hex bytes + \0 , it is the md5 hash of the file
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