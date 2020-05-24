# Directorios
BINARY_DIR=bin

# Opt de compilacion 
CC=gcc
CFLAGS=-std=gnu11  -Wall -Werror -pedantic -Wextra -Wconversion -O3

all: mainBmp mainMP


mainBmp : mainBmp.c simple_bmp.h
	mkdir -p $(BINARY_DIR)
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/mainBmp.o -c mainBmp.c -lm
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/mainBmp   $(BINARY_DIR)/mainBmp.o -lm
	
mainMP : mainMP.c simple_bmp.h
	mkdir -p $(BINARY_DIR)
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/mainMP.o -c mainMP.c -lm -fopenmp
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/mainMP   $(BINARY_DIR)/mainMP.o -lm -fopenmp
	

.PHONY: clean
clean:
	rm  -Rf $(BINARY_DIR)

