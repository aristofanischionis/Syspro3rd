OBJS 	= dropbox_server.o dropbox_client.o Sfunctions.o Cfunctions.o list.o Buffer.o
SOURCE	= src/Server/dropbox_server.c src/Client/dropbox_client.c src/Client/Cfunctions.c src/Server/Sfunctions.c src/Client/Buffer.c 
HEADER  = src/Client/headerfile.h src/Server/headerfile.h src/HeaderFiles/LinkedList.h src/HeaderFiles/Common.h 
OUT  	= dropbox_server dropbox_client
CC		= gcc
FLAGS   = -g -c -Wall
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: $(OUT)

dropbox_server : dropbox_server.o Sfunctions.o list.o
	$(CC) -g dropbox_server.o Sfunctions.o list.o -o dropbox_server

dropbox_client : dropbox_client.o Cfunctions.o Buffer.o list.o
	$(CC) -g  dropbox_client.o Cfunctions.o Buffer.o list.o -l pthread -o dropbox_client

# create/compile the individual files >>separately<< 

Cfunctions.o : src/Client/Cfunctions.c
	$(CC) $(FLAGS) src/Client/Cfunctions.c

Sfunctions.o : src/Server/Sfunctions.c 
	$(CC) $(FLAGS) src/Server/Sfunctions.c

dropbox_client.o : src/Client/dropbox_client.c 
	$(CC) $(FLAGS) src/Client/dropbox_client.c 

Buffer.o : src/Client/Buffer.c 
	$(CC) $(FLAGS) src/Client/Buffer.c 

dropbox_server.o : src/Server/dropbox_server.c 
	$(CC) $(FLAGS) src/Server/dropbox_server.c 

list.o : src/GenericList/list.c 
	$(CC) $(FLAGS) src/GenericList/list.c 

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)

