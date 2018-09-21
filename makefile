all: server client


server: server.o
	gcc  -g server.o -o server

client: client.o
	gcc -g client.o -o client

server.o: server.c
	gcc -c server.c -g -o server.o

client.o: client.c
	gcc -c  client.c -o client.o

testclient:
	./client 192.168.1.17 5544 input.txt  > log.txt

testserver:
	./server 5544 > log.txt

clean: 
	rm -f *.o client server  






