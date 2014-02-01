
CC=gcc
CFLAGS=-g -O0 -march=core2 -mtune=core2 -Wall
LFLAGS= -lImlib2 -lm

maze: maze.h maze.c Makefile
	$(CC) $(CFLAGS) maze.c -o maze $(LFLAGS)

clean:
	rm maze
