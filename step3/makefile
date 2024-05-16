CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =

SRCS = FS.c
OBJS = $(SRCS:.c=.o)
DEPS = include/inode.h include/directory.h include/file.h include/disk_client.h include/disk_server.h

TARGET = FS

.PHONY: all clean

all: $(TARGET)
	$(CC) $(CFLAGS) -o BDS BDS.c
	$(CC) $(CFLAGS) -o FC FC.c
	mv BDS FC FS ./demo

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)