# Syspro3rd

./dropbox_server -p 8081
./dropbox_client -d client1 -p 9091 -w 2 -b 100 -sp 8081 -sip 127.0.1.1
./dropbox_client -d client2 -p 9092 -w 2 -b 100 -sp 8081 -sip 127.0.1.1

## List of things TODO

    *check for big files
    *check with more clients
    *check if working at linux workstations
    *fix leaks
    *comments
    *readme
    *clearup
