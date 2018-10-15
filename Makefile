CC=gcc
CFLAGS=-I. -c -Os 

LD=gcc
LDFLAGS = -L/usr/local/lib
execute: server
	ls -l ./server
	./server

main.o: main.c
	$(CC) $(CFLAGS) -o main.o main.c
cJSON.o: cJSON.c
	$(CC) $(CFLAGS) -o cJSON.o cJSON.c
server: main.o cJSON.o
	$(LD) $(LDFLAGS) -o server main.o cJSON.o -lmicrohttpd


