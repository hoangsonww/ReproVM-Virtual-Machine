CC := gcc
CFLAGS := -std=c99 -O2 -Wall -Wextra -g
LDLIBS :=

SRCS := main.c task.c cas.c util.c
OBJS := $(SRCS:.c=.o)

PARALEL_SRC := reprovm_parallel.c parallel_executor.c

BIN_SERIAL := reprovm
BIN_PARALLEL := reprovm_parallel
CRC32 := crc32_asm

.PHONY: all clean

all: $(BIN_SERIAL) $(BIN_PARALLEL)

$(BIN_SERIAL): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_PARALLEL): $(PARALEL_SRC) task.o cas.o util.o
	$(CC) $(CFLAGS) -o $@ reprovm_parallel.c parallel_executor.c task.o cas.o util.o -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# build assembly CRC32 utility if nasm is available
crc32_asm: crc32_asm.asm
	nasm -f elf64 crc32_asm.asm -o crc32_asm.o
	ld crc32_asm.o -o crc32_asm

clean:
	rm -f *.o $(BIN_SERIAL) $(BIN_PARALLEL) crc32_asm
	rm -rf .reprovm
