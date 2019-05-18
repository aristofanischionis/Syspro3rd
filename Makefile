OBJS 	= dropbox_server.o dropbox_client.o functions.o
SOURCE	= src/Server/dropbox_server.c src/Client/dropbox_client.c src/Client/functions.c
HEADER  = src/Client/headerfile.h
OUT  	= dropbox_server dropbox_client
CC		= gcc
FLAGS   = -g -c -Wall
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: $(OUT)

dropbox_server : src/Server/dropbox_server.c
	$(CC) -Wall -o dropbox_server src/Server/dropbox_server.c

dropbox_client : dropbox_client.o functions.o 
	$(CC) -Wall -o dropbox_client dropbox_client.o functions.o -l pthread

# create/compile the individual files >>separately<< 

functions.o : src/Client/functions.c
	$(CC) $(FLAGS) src/Client/functions.c

dropbox_client.o : src/Client/dropbox_client.c 
	$(CC) $(FLAGS) src/Client/dropbox_client.c 
# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)

