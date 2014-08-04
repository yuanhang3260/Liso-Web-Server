#
# Makefile for Project 1
# Liso HTTP/1.1 Web Server
#
# Hang Yuan <hangyuan@andrew.cmu.edu>
#

CC=gcc
CFLAGS=-Wall -Werror -O2
SOURCE=src
VPATH=$(SOURCE)
OBJECTS = liso.o Utility.o selectEngine.o


LFLAGS=-lssl -lcrypto

default: lisod client sample_server

lisod: $(OBJECTS)
	$(CC) $(OBJECTS) -o lisod $(CFLAGS) $(LFLAGS)

client: src/client.c
	$(CC) $(CFLAGS) $(LFLAGS) $< -o client

sample_server: sample_server.c
	$(CC) $(CLFAGS) $< -o $@

$(SOURCE)/%.o: %.c
	$(CC) $(CLFAGS) $< -o $@

clean:
	rm -f lisod
	rm ./*.o

