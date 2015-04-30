CC=gcc
CFLAGS= -g -Wall

all: test

test: regex.o tools.o test.c
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -o $@ -c $^ $(FLAGS)

clean:
	rm -rf test *.o
