CC=gcc
CFLAGS=-Wall -D NUMBER_OF_MODEMS=1

all: main

main:	main.o SMS.o utf.o
	$(CC) $(CFLAGS) src/cJSON.c main.o src/SMS.o src/utf.o -lzmq -o receive

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

SMS.o: src/SMS.c
	$(CC) $(CFLAGS) -c src/SMS.c -o src/SMS.o

utf.o: src/utf.c
	$(CC) $(CFLAGS) -c src/utf.c -o src/utf.o

clean:
	rm *.o
	rm src/*.o
	rm receive
