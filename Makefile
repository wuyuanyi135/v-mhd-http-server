CC=gcc
CFLAGS=-I. -c

LD=gcc
LDFLAGS = -L/usr/local/lib
execute: server
	./server

main.o: main.c
	$(CC) $(CFLAGS) -o main.o main.c
cJSON.o: cJSON.c
	$(CC) $(CFLAGS) -o cJSON.o cJSON.c
server: main.o cJSON.o
	$(LD) $(LDFLAGS) -o server main.o cJSON.o -lmicrohttpd


