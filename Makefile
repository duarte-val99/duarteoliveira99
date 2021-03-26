###################### MAKEFILE #####################

CC = gcc #compiler
CFLAGS = -W -Wall -ansi -pedantic
CFLAGS2 = -W -Wall

all: ndn

ndn: ndn.o udp.o tcp.o 
	$(CC) -o ndn *.o

ndn.o: ndn.c structs.h
	$(CC) -o ndn.o -c ndn.c $(CFLAGS)

udp.o: udp.c udp.h structs.h
	$(CC) -o udp.o -c udp.c $(CFLAGS2)

tcp.o: tcp.c tcp.h structs.h
	$(CC) -o tcp.o -c tcp.c $(CFLAGS2)

clean:
	rm -f *.o core a.out ndn *~		