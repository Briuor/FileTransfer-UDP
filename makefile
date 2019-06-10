all: ./client/client.c 
	gcc -Wall ./client/client.c -o ./client/client
	gcc -Wall ./server/server.c -o ./server/server
