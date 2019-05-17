#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "headerfile.h"
/*  Print  error  message  and  exit  */
void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

/*  Write ()  repeatedly  until  ’size ’ bytes  are  written  */
int write_all(int fd, void *buff, size_t size)
{
    int sent, n;
    for (sent = 0; sent < size; sent += n)
    {
        if ((n = write(fd, buff + sent, size - sent)) == -1)
            return -1; /*  error  */
    }
    return sent;
}

int Initialisation(char *serverIP, int serverPort)
{
    // Firstly connect to server
    struct sockaddr_in servadd; /*  The  address  of  server  */
    struct hostent *hp;         /*  to  resolve  server  ip */
    int sock, n_read;           /*  socket  and  message  length  */
    struct sockaddr_in *tempClient;
    /*  Step  1:  Get a  socket  */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror_exit("socket"); /*  Step  2:  lookup  server ’s  address  and  connect  there  */
    if ((hp = gethostbyname(serverIP)) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }
    memcpy(&servadd.sin_addr, hp->h_addr, hp->h_length);
    servadd.sin_port = htons(serverPort); /*  set  port  number  */
    servadd.sin_family = AF_INET;         /*  set  socket  type  */
    if (connect(sock, (struct sockaddr *)&servadd, sizeof(servadd)) != 0)
        perror_exit("connect"); /*  Step  3:  send  directory  name +  newline  */
    if (write_all(sock, "LOG_ON\n", 7) == -1)
        perror_exit("LOG_ON");
    if (write_all(sock, "GET_CLIENTS\n", 13) == -1)
        perror_exit("write"); /*  Step  4:  read  back  results  and  send  them  to  stdout  */
    while ((n_read = read(sock, tempClient, sizeof(struct sockaddr_in))) > 0){
        // read one by one the clients and push them in the list which will be given as a parameter with a pointer
        
    }
    
    close(sock);
    return 0;
}