all: ./client/client.c 
	gcc ./client/client.c -o ./client/client
	gcc ./server/server.c -o ./server/server