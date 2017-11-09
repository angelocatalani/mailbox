all: newClient server

newClient: newClient.c client_utilities.c common.h
	   gcc common.h newClient.c client_utilities.c -g -o newClient

server: server.c server_utilities.c hashTable.c utilities.c list.c common.h
	gcc common.h server.c server_utilities.c hashTable.c utilities.c list.c -lpthread -g -o server
