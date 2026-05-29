CC := clang
CFLAGS := -std=c23 -Wall -Wextra -Werror -g
SRC := $(wildcard src/*.c)
BUILD := build
OBJ := $(SRC:src/%.c=$(BUILD)/%.o)
BIN := $(BUILD)/prism

.PHONY: all clean test sanitizer

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) out.S out

test: all
	./tests/run_tests.sh

run: all
	./run.sh

sanitizer: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
sanitizer: clean all

# macOS leak check
leakcheck: all
	@for f in examples/*.c; do \
		echo "=== $$f ==="; \
		leaks --atExit -- ./build/prism $$f -o /tmp/prism_leak.S >/tmp/leakout 2>&1; \
		grep -E "total leaked|ROOT LEAK" /tmp/leakout || echo "  0 leaks"; \
	done
