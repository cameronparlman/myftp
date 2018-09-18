all: server client


server: server.o
	gcc  -g server.o -o server

client: client.o
	gcc -g client.o -o client

server.o: server.c
	gcc -c server.c -o server.o

client.o: client.c
	gcc -c  client.c -o client.o

clean: 
	rm -f *.o client server  






