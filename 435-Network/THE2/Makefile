# Makefile for client and server

CC = gcc
CLIENT = client.c
SERVER = server.c
CFLAGS = -pthread
# setup for system
LIBS =

all: client server

client:	$(OBJCLI)
	$(CC) $(CFLAGS) -o $@ $(CLIENT) $(LIBS)

server:	$(OBJSRV)
	$(CC) $(CFLAGS) -o $@ $(SERVER) $(LIBS)

clean:
	rm client server
