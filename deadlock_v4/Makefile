CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c11 -I./include

BIN_DIR = ../bin
SRCS = src/deadlock_core.c src/banker.c src/detection.c src/recovery.c src/rag.c src/deadlock_cli.c
HEADERS = include/deadlock.h

all: $(BIN_DIR) $(BIN_DIR)/deadlock_cli

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/deadlock_cli: $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(BIN_DIR)/deadlock_cli

.PHONY: all clean
