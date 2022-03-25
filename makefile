CC = gcc
CFLAGS = -Wall -g
prog: main.c yalist.o yastring.o
	$(CC) $(CFLAGS) main.c yastring.o yalist.o -o yaci
yalist.o: yalist.c yalist.h
	$(CC) $(CFLAGS) -c yalist.c -o yalist.o
yastring.o: yastring.c yastring.h
	$(CC) $(CFLAGS) -c yastring.c -o yastring.o
