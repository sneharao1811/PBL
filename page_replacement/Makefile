CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c11 -I./include

BIN_DIR = ../bin
SRCS = src/page_core.c src/fifo.c src/lru.c src/optimal.c src/page_cli.c
HEADERS = include/page_replacement.h

TARGET = $(BIN_DIR)/page_replacement_cli

all: $(BIN_DIR) $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all clean
