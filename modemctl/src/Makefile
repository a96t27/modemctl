CFLAGS += -Wall -Wextra -pedantic
CPPFLAGS += -I./include
LDLIBS += -largp -lcjson

BIN := bin/modemctl
BUILD_DIR := build

SRCS := $(shell find src -name '*.c')
OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BUILD_DIR) $(BIN)

run: all
	./$(BIN)