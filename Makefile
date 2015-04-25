CC=gcc
CFLAGS= -g -Wall

all: regex

regex: regex.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf regex
