CC=gcc
CFLAGS= -g -Wall

all: test

test: regex.o tools.o test.c
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf test *.o
