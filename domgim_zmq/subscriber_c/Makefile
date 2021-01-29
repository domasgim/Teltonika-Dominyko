CC=gcc
CFLAGS=-Wall -D NUMBER_OF_MODEMS=1

all: encode

encode:	encode.o pdu.o
	$(CC) $(CFLAGS) cJSON.c encode.o pdu.o -lzmq -o send

encode.o: encode.c
	$(CC) $(CFLAGS) -c encode.c

pdu.o:	pdu.c
	$(CC) $(CFLAGS) -c pdu.c

clean:
	rm *.o
	rm send
