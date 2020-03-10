CC=gcc
CFLAGS=-O0 -g -Wall

all: main

main: main.o msgpack.o
	$(CC) $^ -o $@

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	rm -f *.o main
