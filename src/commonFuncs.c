#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include "HeaderFiles/Common.h"

int bind_on_port(int sock, short port)
{
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);
	return bind(sock, (struct sockaddr *)&server, sizeof(server));
}

void handle_signal_action(int sig_number)
{
	if (sig_number == SIGINT)
	{
		printf("SIGINT was catched!\n");
		shutdown_properly(EXIT_SUCCESS);
	}
	else if (sig_number == SIGPIPE)
	{
		printf("SIGPIPE was catched!\n");
		shutdown_properly(EXIT_SUCCESS);
	}
}

int setup_signals()
{
	struct sigaction sa;
	sa.sa_handler = handle_signal_action;
	if (sigaction(SIGINT, &sa, 0) != 0)
	{
		perror("sigaction()");
		return -1;
	}
	if (sigaction(SIGPIPE, &sa, 0) != 0)
	{
		perror("sigaction()");
		return -1;
	}

	return 0;
}

void appendString(char **new_str, char *str1, char *str2)
{
	*new_str[0] = '\0'; // ensures the memory is an empty string
	strcat(*new_str, str1);
	strcat(*new_str, str2);
}

// Returns hostname for the local computer
void checkHostName(int hostname)
{
	if (hostname == -1)
	{
		perror("gethostname");
		exit(1);
	}
}

// Returns host information corresponding to host name
void checkHostEntry(struct hostent *hostentry)
{
	if (hostentry == NULL)
	{
		perror("gethostbyname");
		exit(1);
	}
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer)
{
	if (NULL == IPbuffer)
	{
		perror("inet_ntoa");
		exit(1);
	}
}

int make_socket(char *myIP, int port)
{
	int socketD;
	int opt = 1;
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = inet_addr(myIP);
	sock_addr.sin_port = htons(port);
	if ((socketD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(socketD, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	if (bind(socketD, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	return socketD;
}

int connect_to_socket(char *myIP, int port)
{
	struct sockaddr_in sock_addr;
	int socketD = 0;
	if ((socketD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		pthread_exit(0);
	}
	memset(&sock_addr, '0', sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = inet_addr(myIP);
	sock_addr.sin_port = htons(port);
	// connect
	int con = connect(socketD, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in));
	if (con == 0)
		printf("Client Connected\n");
	else
		printf("Error in Connection\n");

	return socketD;
}

char *strremove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if (len > 0)
    {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL)
        {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}