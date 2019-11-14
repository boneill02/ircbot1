include config.mk

BIN=ib1

SRC=$(patsubst src/%.c, src/%.o, $(wildcard src/*.c))

all: $(BIN)

run-debug: $(BIN)
	gdb ./$(BIN)

$(BIN): $(SRC)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -f $(wildcard src/*.o) $(BIN)
