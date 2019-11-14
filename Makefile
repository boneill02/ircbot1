include config.mk

BIN=ib1

SRC=$(patsubst %.c, %.o, $(wildcard *.c))

all: $(BIN)

run-debug: $(BIN)
	gdb ./$(BIN)

$(BIN): $(SRC)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm $(wildcard *.o) $(BIN)
