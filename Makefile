OBJS 	= main.o dropbox_server.o dropbox_client.o
SOURCE	= main.c 
HEADER  = HeaderFiles/Input.h
OUT  	= dropbox_server dropbox_client
CC		= gcc
FLAGS   = -g -c -Wall
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: $(OUT)

dropbox_server : dropbox_server.o
	$(CC) -Wall -o dropbox_server src/Server/server.c

dropbox_client : dropbox_client.o
	$(CC) -Wall -o dropbox_client dropbox_client.c -pthread

# create/compile the individual files >>separately<< 
main.o: main.c
	$(CC) $(FLAGS) main.c

Input.o: src/Frontend/Input.c
	$(CC) $(FLAGS) src/Frontend/Input.c

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)

