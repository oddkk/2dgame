SRC_DIR := src/

CFLAGS:=-Wall -g -I$(SRC_DIR)
LDFLAGS:=-lX11 -lGL -lGLU -ldl

SRC=$(shell find $(SRC_DIR) -name *.c)
HEADERS:=$(shell find $(SRC_DIR) -name *.h)

all: game

game: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $@

clean:
	-rm game
.PHONY: clean
