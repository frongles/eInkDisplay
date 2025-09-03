CC = gcc
CFLAGS = -Wall -g -D_POSIX_C_SOURCE=200112L -D_GNU_SOURCE -Iinclude
DEPFLAGS = -MMD -MP

SRC_PATH = ./src
OBJ_PATH = ./obj
BIN_PATH = ./bin
TARG = $(BIN_PATH)/test

SRC := $(wildcard $(SRC_PATH)/*.c)

OBJ := $(patsubst $(SRC_PATH)/%.c, $(OBJ_PATH)/%.o, $(SRC))

DEPS := $(OBJ:.o=.d)
$(shell mkdir -p $(OBJ_PATH) $(BIN_PATH))

all: $(TARG)

$(TARG): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

rebuild: clean all

run: all
	$(TARG)

clean:
	rm -f $(OBJ_PATH)/*.o $(OBJ_PATH)/*.d $(TARG)