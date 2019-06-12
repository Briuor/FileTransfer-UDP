all: 
	gcc -Wall ./cliente_baixa/cliente_baixa.c -o ./cliente_baixa/client
	gcc -Wall ./cliente_envia/cliente_envia.c -o ./cliente_envia/client
	gcc -Wall ./server/server.c -o ./server/server
