CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =

SRCS = main.c
OBJS = $(SRCS:.c=.o)
DEPS = inode.h directory.h file.h

TARGET = myprogram

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)