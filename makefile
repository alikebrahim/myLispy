CC = cc
CFLAGS = -std=c99 -Wall 
LDFLAGS = -ledit -lm
SRC = parsing.c mpc.c
OBJ = $(SRC:.c=.o)
BIN = parsing

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
