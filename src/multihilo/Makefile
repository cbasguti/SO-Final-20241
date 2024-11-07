# Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -Wall
LIBS = -lpthread
OBJS = multiserver.o wclient.o request.o io_helper.o 

.SUFFIXES: .c .o 

all: multiserver wclient spin.cgi

multiserver: multiserver.o request.o io_helper.o
	$(CC) $(CFLAGS) -o multiserver multiserver.o request.o io_helper.o $(LIBS)

wclient: wclient.o io_helper.o
	$(CC) $(CFLAGS) -o wclient wclient.o io_helper.o

spin.cgi: spin.c
	$(CC) $(CFLAGS) -o spin.cgi spin.c

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJS) multiserver wclient spin.cgi
