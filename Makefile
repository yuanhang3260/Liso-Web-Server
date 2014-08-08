#
# Makefile for Project 1
# Liso HTTP/1.1 Web Server
#
# Hang Yuan <hangyuan@andrew.cmu.edu>
#

CC=gcc
CFLAGS=-Wall -Werror -O2
SOURCE_DIR=src
OBJ_DIR=lib
VPATH=$(SOURCE)
OBJECTS = $(OBJ_DIR)/liso.o \
          $(OBJ_DIR)/Utility.o \
          $(OBJ_DIR)/selectEngine.o

S_OBJ = $(OBJ_DIR)/sample_server.o \
        $(OBJ_DIR)/Utility.o \
        $(OBJ_DIR)/selectEngine.o


LFLAGS=-lssl -lcrypto

default: lisod client sample_server

lisod: $(OBJECTS)
	$(CC) $(OBJECTS) -o lisod $(CFLAGS)

client: src/client.c
	$(CC) $(CFLAGS) $< -o client

sample_server: $(S_OBJ)
	$(CC) $(S_OBJ) -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS) 

clean:
	rm -rf lisod client sample_server
	rm -rf $(OBJ_DIR)/*.o

