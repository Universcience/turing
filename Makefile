CFLAGS=-Wall -Wextra -g
LDFLAGS=-lncurses

BIN=turing

turing: main.c turing.c

.PHONY: all clean

all: ${BIN}

clean:
	-rm -f ${BIN}
