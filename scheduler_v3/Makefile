CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -I./include

BIN_DIR = ../bin

all: $(BIN_DIR) $(BIN_DIR)/scheduler_v3

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/scheduler_v3: src/scheduler.c include/scheduler.h
	$(CC) $(CFLAGS) -o $@ src/scheduler.c

clean:
	rm -f $(BIN_DIR)/scheduler_v3
