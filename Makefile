# Fraser Crumpler 21 Dec 20024

CC = gcc
CFLAGS = -Wall -g

TARGET_NAME := testDisplay

TARGET := bin/$(TARGET_NAME)

all: bin/TARGET
TARGET: bin/TARGET

bin/TARGET: obj/TARGET.o
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $^


clean:
	rm *.o


