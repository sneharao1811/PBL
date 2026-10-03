CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2

all: simulator_v1

simulator_v1: simulator_v1.c
	$(CC) $(CFLAGS) -o simulator_v1 simulator_v1.c

clean:
	rm -f simulator_v1 simulator.log
