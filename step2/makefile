CC = gcc
CFLAGS = -Wall -Wextra -g

all: program

program: main.o
	$(CC) $(CFLAGS) -o program main.o

main.o: main.c inode.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f program main.o