# Fraser Crumpler 21 Dec 20024

CC = gcc
CFLAGS = -Wall -g

SRC_PATH = src
OBJ_PATH = obj
BIN_PATH = bin

TARGET_NAME := testDisplay
SRC_NAMES := testDisplay eInkTools spiTools gpioTools stb_truetype

SRC = $(addprefix $(SRC_PATH)/, $(addsuffix .c, $(SRC_NAMES)))
OBJ = $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(SRC_NAMES)))
TARGET := $(BIN_PATH)/$(TARGET_NAME)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(CFLAGS) -o $@ -c $^


clean:
	rm *.o


