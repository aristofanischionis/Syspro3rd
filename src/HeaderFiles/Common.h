#ifndef COMMON_HEADER
#define COMMON_HEADER

int bind_on_port(int sock, short port);
void shutdown_properly(int code);
void handle_signal_action(int sig_number);
int setup_signals();
void appendString(char **new_str, char *str1, char *str2);

#endif