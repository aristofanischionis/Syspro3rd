OBJS 	= dropbox_server.o dropbox_client.o
SOURCE	= src/Server/dropbox_server.c src/Client/dropbox_client.c
# HEADER  = HeaderFiles/Input.h
OUT  	= dropbox_server dropbox_client
CC		= gcc
FLAGS   = -g -c -Wall
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: $(OUT)

dropbox_server : src/Server/dropbox_server.c
	$(CC) -Wall -o dropbox_server src/Server/dropbox_server.c

dropbox_client : src/Client/dropbox_client.c
	$(CC) -Wall -o dropbox_client src/Client/dropbox_client.c -pthread

# create/compile the individual files >>separately<< 


# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)

