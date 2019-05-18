#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/stat.h>
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

int main(int argc, char *argv[])
{
    char *localhost;
    localhost = malloc(20);
    strcpy(localhost, "127.0.0.1");
    int port = 0;
    if (argc != 3)
    {
        fprintf(stderr, "Wrong number of arguments given %d \n", argc);
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[2]);
    // Two buffers for message communication
    char buffer1[256], buffer2[256];
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
        printf("Error in server creating\n");
    else
        printf("Server Created\n");

    struct sockaddr_in my_addr, peer_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;

    // This ip address will change according to the machine
    my_addr.sin_addr.s_addr = inet_addr(localhost);

    my_addr.sin_port = htons(port);

    if (bind(server, (struct sockaddr *)&my_addr, sizeof(my_addr)) == 0)
        printf("Binded Correctly\n");
    else
        printf("Unable to bind\n");

    if (listen(server, 3) == 0)
        printf("Listening ...\n");
    else
        printf("Unable to listen\n");

    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_in);

    // while loop is iterated infinitely to
    // accept infinite connection one by one
    while (1)
    {
        int acc = accept(server, (struct sockaddr *)&peer_addr, &addr_size);
        printf("Connection Established\n");
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(peer_addr.sin_addr), ip, INET_ADDRSTRLEN);

        // "ntohs(peer_addr.sin_port)" function is
        // for finding port number of client
        printf("connection established with IP : %s and PORT : %d\n",
               ip, ntohs(peer_addr.sin_port));

        if(recv(acc, buffer2, 256, 0)  == 0)
            close(server);

        printf("Client : %s\n", buffer2);
        strcpy(buffer1, "Hello");
        send(acc, buffer1, 256, 0);
    }
    return 0;
}