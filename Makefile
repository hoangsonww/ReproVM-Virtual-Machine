CC = gcc
CFLAGS = -std=c99 -O2 -Wall -Wextra -g

SRCS = main.c task.c cas.c util.c
OBJS = $(SRCS:.c=.o)
TARGET = reprovm

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
