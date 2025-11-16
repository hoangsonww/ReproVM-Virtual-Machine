CC := gcc
CFLAGS := -std=c99 -O2 -Wall -Wextra -g
LDLIBS := -lpthread

# Core sources
CORE_SRCS := task.c cas.c util.c

# Production-ready modules
PROD_SRCS := logger.c config.c metrics.c error_handling.c security.c \
             signal_handler.c health_check.c

# All common sources
COMMON_SRCS := $(CORE_SRCS) $(PROD_SRCS)
COMMON_OBJS := $(COMMON_SRCS:.c=.o)

# Entry points
SERIAL_SRC := main.c
PARALLEL_SRCS := reprovm_parallel.c parallel_executor.c

# Binaries
BIN_SERIAL := reprovm
BIN_PARALLEL := reprovm_parallel
CRC32 := crc32_asm

.PHONY: all clean test help install uninstall coverage

all: $(BIN_SERIAL) $(BIN_PARALLEL)

# Serial binary with all production modules
$(BIN_SERIAL): $(SERIAL_SRC:.c=.o) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Parallel binary with all production modules
$(BIN_PARALLEL): $(PARALLEL_SRCS:.c=.o) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Build assembly CRC32 utility if nasm is available
crc32_asm: crc32_asm.asm
	@command -v nasm >/dev/null 2>&1 && { \
		nasm -f elf64 crc32_asm.asm -o crc32_asm.o; \
		ld crc32_asm.o -o crc32_asm; \
	} || echo "NASM not found, skipping crc32_asm build"

# Run tests
test: all
	@echo "Running tests..."
	@cd tests && bash test_all.sh

# Code coverage build
coverage: CFLAGS += --coverage -fprofile-arcs -ftest-arcs
coverage: LDLIBS += --coverage
coverage: clean all
	@echo "Built with coverage instrumentation"

# Install binaries
install: all
	@echo "Installing ReproVM..."
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(BIN_SERIAL) $(DESTDIR)/usr/local/bin/
	install -m 755 $(BIN_PARALLEL) $(DESTDIR)/usr/local/bin/
	@echo "Installation complete"

# Uninstall
uninstall:
	rm -f $(DESTDIR)/usr/local/bin/$(BIN_SERIAL)
	rm -f $(DESTDIR)/usr/local/bin/$(BIN_PARALLEL)
	@echo "Uninstallation complete"

# Help target
help:
	@echo "ReproVM Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build both serial and parallel binaries (default)"
	@echo "  clean      - Remove built files and cache"
	@echo "  test       - Run test suite"
	@echo "  coverage   - Build with code coverage instrumentation"
	@echo "  install    - Install binaries to /usr/local/bin"
	@echo "  uninstall  - Remove installed binaries"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Environment variables:"
	@echo "  CC         - C compiler (default: gcc)"
	@echo "  CFLAGS     - Compiler flags"
	@echo "  DESTDIR    - Installation prefix"

# Clean build artifacts and cache
clean:
	rm -f *.o $(BIN_SERIAL) $(BIN_PARALLEL) crc32_asm *.gcda *.gcno *.gcov
	rm -rf .reprovm coverage.xml
	@echo "Clean complete"
