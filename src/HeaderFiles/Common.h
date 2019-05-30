#ifndef COMMON_HEADER
#define COMMON_HEADER

int bind_on_port(int sock, short port);
void shutdown_properly(int code);
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

#endif