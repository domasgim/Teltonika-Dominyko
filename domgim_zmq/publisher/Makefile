CC=gcc
CFLAGS=-Wall -D NUMBER_OF_MODEMS=1

all: main

main:	main.o SMS.o utf.o
	$(CC) $(CFLAGS) cJSON.c main.o SMS.o utf.o -lzmq -o receive

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

SMS.o: SMS.c
	$(CC) $(CFLAGS) -c SMS.c

utf.o: utf.c
	$(CC) $(CFLAGS) -c utf.c

clean:
	rm *.o
	rm receive
