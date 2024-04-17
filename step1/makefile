CC = gcc
CFLAGS = -Wall -Wextra

all: BDS BDC_random BDC_command

ForkCopy: BDC_command.c
	$(CC) $(CFLAGS) -o BDC_command BDC_command.c

MyCopy: BDC_random.c
	$(CC) $(CFLAGS) -o BDC_random BDC_random.c

PipeCopy: BDS.c
	$(CC) $(CFLAGS) -o BDS BDS.c

clean:
	rm -f BDS BDC_random BDC_command