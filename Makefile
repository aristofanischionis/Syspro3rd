OBJS 	= dropbox_server.o dropbox_client.o Sfunctions.o Cfunctions.o list.o Buffer.o commonFuncs.o
SOURCE	= src/Server/dropbox_server.c src/Client/dropbox_client.c src/Client/Cfunctions.c src/Server/Sfunctions.c src/Client/Buffer.c src/commonFuncs.c
HEADER  = src/Client/headerfile.h src/Server/headerfile.h src/HeaderFiles/LinkedList.h src/HeaderFiles/Common.h 
OUT  	= dropbox_server dropbox_client
CC		= gcc
FLAGS   = -g -c -Wall
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: $(OUT)

dropbox_server : dropbox_server.o Sfunctions.o list.o commonFuncs.o
	$(CC) -g dropbox_server.o Sfunctions.o list.o commonFuncs.o -o dropbox_server

dropbox_client : dropbox_client.o Cfunctions.o Buffer.o list.o commonFuncs.o
	$(CC) -g  dropbox_client.o Cfunctions.o Buffer.o list.o commonFuncs.o -l pthread -o dropbox_client

# create/compile the individual files >>separately<< 

Cfunctions.o : src/Client/Cfunctions.c src/commonFuncs.c src/Client/headerfile.h
	$(CC) $(FLAGS) src/Client/Cfunctions.c src/commonFuncs.c 

Sfunctions.o : src/Server/Sfunctions.c src/commonFuncs.c src/Server/headerfile.h
	$(CC) $(FLAGS) src/Server/Sfunctions.c src/commonFuncs.c

dropbox_client.o : src/Client/dropbox_client.c src/commonFuncs.c src/Client/headerfile.h
	$(CC) $(FLAGS) src/Client/dropbox_client.c src/commonFuncs.c

Buffer.o : src/Client/Buffer.c src/Client/headerfile.h
	$(CC) $(FLAGS) src/Client/Buffer.c 

dropbox_server.o : src/Server/dropbox_server.c src/commonFuncs.c src/Server/headerfile.h
	$(CC) $(FLAGS) src/Server/dropbox_server.c src/commonFuncs.c

list.o : src/List/list.c src/HeaderFiles/LinkedList.h
	$(CC) $(FLAGS) src/List/list.c 

commonFuncs.o : src/commonFuncs.c src/HeaderFiles/Common.h 
	$(CC) $(FLAGS) src/commonFuncs.c

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)

