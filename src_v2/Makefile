CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -I./include
LDFLAGS ?= -lrt -pthread

BIN_DIR = ../bin

all: $(BIN_DIR) $(BIN_DIR)/logger_proc $(BIN_DIR)/core_proc $(BIN_DIR)/ui_proc $(BIN_DIR)/simulator_v2

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/logger_proc: logger/logger_main.c include/protocol.h
	$(CC) $(CFLAGS) -o $@ logger/logger_main.c $(LDFLAGS)

$(BIN_DIR)/core_proc: core/core_main.c include/protocol.h
	$(CC) $(CFLAGS) -o $@ core/core_main.c $(LDFLAGS)

$(BIN_DIR)/ui_proc: ui/ui_main.c include/protocol.h
	$(CC) $(CFLAGS) -o $@ ui/ui_main.c $(LDFLAGS)

$(BIN_DIR)/simulator_v2: launcher/supervisor.c include/protocol.h
	$(CC) $(CFLAGS) -o $@ launcher/supervisor.c $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR) ../simulator.log
