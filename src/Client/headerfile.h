
#ifndef CLIENT_HEADER
#define CLIENT_HEADER

int Initialisation(char *serverIP, int serverPort);
void *threadsWork(void *args);
void *Mainthread(void *args);
void sendLogOn(int sock);
void sendGetClients(int sock);
void sendLogOff(int sock);
void tokenizeClientList(char *input);
char *strremove(char *str, const char *sub);
void deleteFromList(char *input);
void insertInClientList(char *input);
void sendFileList(char *dirName, int clientSocket);
void sendFileContents(char *pathName, int socketSD, char* version);
void putRequestsInBuffer();
void sendFile(char *pathName, char *version, int socketSD);
void readFileList(char *source, char *IPsender, int portSender);
void readFile(char *source, int socketSD, char *fullPath);
int read_from_client1(int socketD, char *dir);
void terminating();

struct args_MainThread
{
    char serverIP[25];
    char myIP[25];
    int serverPort;
    int clientPort;
    char dirName[512];
    int bufSize;
};

struct args_Workers
{
    char myIP[25];
    int myPort;
    char dirName[512];
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
    struct buffer_entry *elements;
    int start;
    int end;
    int count;
} Buffer;

struct buffer_entry retrieve();
void put(buffer_entry data);
void init(int bufSize);
void destroy();

#endif