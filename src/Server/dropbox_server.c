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

int main(int argc, char *argv[])
{
    Node *headList = NULL;
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
    char *message;
    message = malloc(256);
    // select
    int opt = 1;
    int *client_socket;
    client_socket = malloc(30 * sizeof(int));
    int master_socket, addrlen, new_socket,
        max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    char *buffer; //data buffer of 1K
    buffer = malloc(BUFSIZ + 1);

    //set of socket descriptors
    fd_set readfds;
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", port);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    while (1)
    {
        //clear the socket set
        FD_ZERO(&readfds);
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        //add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            //highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n ", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            //send new connection greeting message
            // sprintf(message, "Welcome client with id %d\n", new_socket);
            // if (send(new_socket, message, strlen(message), 0) != strlen(message))
            // {
            //     perror("send");
            // }

            // puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }
        //else its some IO operation on some other socket
        // -------------------------------> mpalitsa
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read(sd, buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                }
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    // send(sd, buffer, strlen(buffer), 0);
                    if (strstr(buffer, "LOG_ON") != NULL)
                    {
                        int cliSocket;
                        cliSocket = logOn(&headList, buffer, sd, max_clients, &client_socket, &max_sd, readfds);
                        recv(sd, buffer, 1024, 0);
                        printf("buffer -------> %s\n", buffer);
                        if(!strcmp(buffer, "GET_CLIENTS")){
                            getClients(&headList, cliSocket); 
                            recv(sd, buffer, 1024, 0);
                            if(strstr(buffer, "LOG_OFF") != NULL){
                                logOff(&headList, buffer, cliSocket, max_clients, client_socket);
                                recv(sd, buffer, 1024, 0);
                                printf("1buffer read --> %s\n", buffer);
                            }
                            else {
                                printf("2buffer read --> %s\n", buffer);
                            }
                        }
                        else{
                            printf("3buffer read --> %s\n", buffer);
                        }
                    }
                    // else if (!strcmp(buffer, "GET_CLIENTS"))
                    // {
                    //     getClients(&headList, sd);
                    // }
                    // else if (!strcmp(buffer, "LOG_OFF"))
                    // {
                    //     logOff(&headList, buffer, cliSocket, max_clients, client_socket);
                    // }
                    // else {
                    //     printf("I read something else: %s \n", buffer);
                    // }


                }
            }

            // ->>>>>>>>>>>>>>>>
        }

    }
    return 0;
}