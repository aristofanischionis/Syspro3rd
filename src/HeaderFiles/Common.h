#ifndef COMMON_HEADER
#define COMMON_HEADER
#include <netdb.h>

void handle_signal_action(int sig_number);
int setup_signals();
void appendString(char **new_str, char *str1, char *str2);
int make_socket(char *myIP, int port);
int connect_to_socket(char *myIP, int port);
void checkHostName(int hostname);
void checkHostEntry(struct hostent *hostentry);
void checkIPbuffer(char *IPbuffer);
int make_socket(char *myIP, int port);
int connect_to_socket(char *myIP, int port);
char *strremove(char *str, const char *sub);
void findFiles(char *source, int indent, char **result, int *NumOfFiles);
long long countSize(char *filename);
char *calculateMD5hash(char *pathname);


#endif