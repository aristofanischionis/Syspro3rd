#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include "HeaderFiles/Common.h"

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
	if (con == 0){
		printf("Client Connected\n");
	}
	else{
		printf("Error in Connection\n");
		exit(1);
	}

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

void findFiles(char *source, int indent, char **result, int *NumOfFiles)
{
	DIR *dir;
	struct dirent *entry;
	char path[1025];
	struct stat info;
	char *temp;
	temp = malloc(60);
	if ((dir = opendir(source)) == NULL)
		perror("opendir() error");
	else
	{
		while ((entry = readdir(dir)) != NULL)
		{
			if (entry->d_name[0] != '.')
			{
				strcpy(path, source);
				strcat(path, "/");
				strcat(path, entry->d_name);
				if (stat(path, &info) != 0)
				{
					fprintf(stderr, "stat() error on %s: %s\n", path, strerror(errno));
				}
				else if (S_ISDIR(info.st_mode))
				{
					// it is a directory
					findFiles(path, indent + 1, result, NumOfFiles);
				}
				else if (S_ISREG(info.st_mode))
				{
					// it is a file
					(*NumOfFiles) = (*NumOfFiles) + 1;
					char *version, *str1, *str2;
					str1 = malloc(BUFSIZ);
					str2 = malloc(60);
					version = malloc(33);
					strcpy(version, calculateMD5hash(path));
					sprintf(temp, "< %s , %s > ", path, version);
					strcpy(str1, *result);
					strcpy(str2, temp);
					appendString(result, str1, str2);
					free(version);
					free(str1);
					free(str2);
				}
			}
		}
		closedir(dir);
	}
}

long long countSize(char *filename)
{
    struct stat sb;
    if (stat(filename, &sb) == -1)
    {
        perror("stat");
        exit(1);
    }

    // printf("File size: %lld bytes\n", (long long)sb.st_size);
    return ((long long)sb.st_size);
}

char *calculateMD5hash(char *pathname)
{
    char *result;
    char *fileWithHash;
    char *temp;
    char *command;
    FILE *fp;
    result = malloc(33);
    temp = malloc(strlen(pathname) + 1);
    fileWithHash = malloc(strlen(pathname) + 5);
    strcpy(result, "");
    // call the script to make the file with the hash then read it from there
    sprintf(fileWithHash, "%s.md5", pathname);

    command = malloc(strlen(pathname) + strlen(fileWithHash) + 12);
    sprintf(command, "md5sum %s > %s", pathname, fileWithHash);
    system(command);
    free(command);

    // now that the file is ready with the checksum
    // open the file and extract the hash

    fp = fopen(fileWithHash, "r");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "%s %s", result, temp);

    fclose(fp);
    remove(fileWithHash);
    free(temp);
    free(fileWithHash);
    return result;
}