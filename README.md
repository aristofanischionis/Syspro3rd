# Syspro3rd

./dropbox_server -p 8081
./dropbox_client -d client1 -p 9091 -w 2 -b 100 -sp 8081 -sip 127.0.1.1
./dropbox_client -d client2 -p 9092 -w 2 -b 100 -sp 8081 -sip 127.0.1.1

## List of things TODO

    *send files in chuncks
    *Make a parent folder to put the backups from other clients
    *Fix LogOFF and signaling
    *memory management frees, threads handling