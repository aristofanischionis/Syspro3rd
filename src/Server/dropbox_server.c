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
#include "../HeaderFiles/LinkedList.h"
#include "headerfile.h"

void perror_exit(char *msg);
void sanitize(char *str);

/*  it  would  be  very  bad  if  someone  passed  us an  dirname  like* "; rm  *"   and  we  naively  created  a  command   "ls ; rm  *".* So..we  remove  everything  but  slashes  and  alphanumerics .*/
void sanitize(char *str)
{
    char *src, *dest;
    for (src = dest = str; *src; src++)
        if (*src == '/' || isalnum(*src))
            *dest++ = *src;
    *dest = '\0';
}
/*  Print  error  message  and  exit  */
void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
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

int read_from_client(int filedes)
{
    char buffer[BUFSIZ];
    int nbytes;

    nbytes = read(filedes, buffer, BUFSIZ);
    if (nbytes < 0)
    {
        /* Read error. */
        perror("read");
        exit(EXIT_FAILURE);
    }
    else if (nbytes == 0)
        /* End-of-file. */
        return -1;
    else
    {
        /* Data read. */
        fprintf(stderr, "Server: got message: '%s'\n", buffer);
        if (strstr(buffer, "LOG_ON") != NULL)
        {
            int cliSocket;
            char *IP = malloc(25);
            strcpy(IP, "");
            int port = 0;
            cliSocket = logOn(&headList, buffer, sd, max_clients, &client_socket, &max_sd, readfds, &IP, &port);
        }
        else if (!strcmp(buffer, "GET_CLIENTS"))
        {
            getClients(&headList, cliSocket, IP, port);
        }

        else if (!strcmp(buffer, "LOG_OFF"))
        {
            printf("buffer --> %s\n", buffer);
            // logOff(&headList, buffer, cliSocket, max_clients, client_socket);
        }
        else
        {
            printf("I read something else: %s \n", buffer);
        }
        return 0;
    }
}

int main(int argc, char *argv[])
{
    Node *headList = NULL;
    int port = 0;
    if (argc != 3)
    {
        fprintf(stderr, "Wrong number of arguments given %d \n", argc);
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[2]);

    int opt = 1;
    int *client_socket;
    client_socket = malloc(30 * sizeof(int));
    int master_socket, addrlen, new_socket,
        max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    char *buffer; //data buffer of 1K
    buffer = malloc(BUFSIZ + 1);
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;
    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    checkHostName(hostname);

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);

    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
    int sock;
    fd_set active_fd_set, read_fd_set;
    int i;
    struct sockaddr_in clientname;
    size_t size;

    /* Create the socket and set it up to accept connections. */
    sock = make_socket(port);
    if (listen(sock, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* Initialize the set of active sockets. */
    FD_ZERO(&active_fd_set);
    FD_SET(sock, &active_fd_set);

    while (1)
    {
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i < FD_SETSIZE; ++i)
            if (FD_ISSET(i, &read_fd_set))
            {
                if (i == sock)
                {
                    /* Connection request on original socket. */
                    int new;
                    size = sizeof(clientname);
                    new = accept(sock,
                                 (struct sockaddr *)&clientname,
                                 &size);
                    if (new < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stderr,
                            "Server: connect from host %s, port %hd.\n",
                            inet_ntoa(clientname.sin_addr),
                            ntohs(clientname.sin_port));
                    FD_SET(new, &active_fd_set);
                }
                else
                {
                    /* Data arriving on an already-connected socket. */
                    if (read_from_client(i) < 0)
                    {
                        close(i);
                        FD_CLR(i, &active_fd_set);
                    }
                }
            }
    }
}