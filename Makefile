CC?=gcc
DBG?=gdb

CSRC:=$(wildcard source/*.c)
COBJ:=$(patsubst source/%.c, build/%.c.o, $(CSRC))

CFLAGS+=-Og -g -Wall -Werror -Wextra -Wno-error=switch -Iinclude/ -c -MMD
LFLAGS+=

BIN:=build/jcl

build/%.c.o: source/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(BIN): $(COBJ)
	$(CC) $(LFLAGS) $(COBJ) -o $@

.PHONY: all run debug clean

all: $(BIN)

run: $(BIN)
	./$(BIN) tests/test.jcl

debug: $(BIN)
	$(DBG) $(BIN)

clean:
	rm -rf build/

-include build/*.c.d
