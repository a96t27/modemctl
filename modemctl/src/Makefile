CFLAGS += -Wall -Wextra -pedantic
CPPFLAGS += -I./include
LDFLAGS += -largp
BIN = bin/modemctl
BUILD_DIR := build
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=bin/%.o)

.PHONY: clean all objects run

all: $(BIN)


$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

build: $(BIN)

objects: $(OBJS)

bin/%.o: src/%.c
	mkdir -p bin/
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c -o $@ $^

clean:
	${RM} $(BIN) $(OBJS)

run: build
	$(BIN)
