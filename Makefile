
CC=gcc
CFLAGS=-g -O3 -march=core2 -mtune=core2 -Wall
LFLAGS= -lImlib2 -lm -lpthread

maze: maze.h maze.c Makefile
	$(CC) $(CFLAGS) maze.c -o maze $(LFLAGS)

clean:
	rm maze
