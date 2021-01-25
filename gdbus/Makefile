.PHONY: all

CC=gcc
CFLAGS=-Wall -D NUMBER_OF_MODEMS=1

FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0)

all: sms_send sms_receive

sms_send: sms_send.o smsprocess.o pdu.o
	gcc -o $@ $^ $(FLAGS)

sms_receive: sms_receive.o smsprocess.o SMS.o utf.o
	gcc -o $@ $^ $(FLAGS)

sms_send.o: sms_send.c
	gcc -o $@ $^ -c $(FLAGS)

sms_receive.o: sms_receive.c
	gcc -o $@ $^ -c $(FLAGS)

smsprocess.o: smsprocess.c
	gcc -o $@ $^ -c $(FLAGS)

SMS.o: SMS.c
	$(CC) $(CFLAGS) -c SMS.c

utf.o: utf.c
	$(CC) $(CFLAGS) -c utf.c

pdu.o:	pdu.c
	$(CC) $(CFLAGS) -c pdu.c

clean:
	rm -f *.o sms_send sms_receive

# gen stub
gen:
	gdbus-codegen --generate-c-code smsprocess --c-namespace SMSProcess --interface-prefix com.smsprocess. com.smsprocess.GDBUS.xml