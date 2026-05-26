CC := clang
CFLAGS := -std=c23 -Wall -Wextra -Werror -g
SRC := $(wildcard src/*.c)
BUILD := build
OBJ := $(SRC:src/%.c=$(BUILD)/%.o)
BIN := $(BUILD)/prism

.PHONY: all clean test

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) out.S out

test:
	@echo "test wired in §8"