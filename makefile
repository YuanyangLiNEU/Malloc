#Sample Makefile for Malloc
CC=gcc
CFLAGS=-g -O0 -fPIC

all: clean check

clean:
	rm -rf libmalloc.so malloc.o util.o free.o realloc.o calloc.o test1 test1.o test1-1 test1-1.o

libmalloc.so: util.o malloc.o free.o realloc.o calloc.o
	$(CC) $(CFLAGS) -shared -Wl,--unresolved-symbols=ignore-all util.o malloc.o free.o realloc.o calloc.o -o $@ -lpthread

test1: test1.o
	$(CC) $(CFLAGS) $< -o $@ -lpthread

test1-1: test1-1.o
	$(CC) $(CFLAGS) $< -o $@ -lpthread

# For every XYZ.c file, generate XYZ.o.
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@  -lpthread

check:	libmalloc.so test1 test1-1
	LD_PRELOAD=`pwd`/libmalloc.so ./test1-1
	#LD_PRELOAD=`pwd`/libmalloc.so ./test1
	#./test1-1

dist:
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar
