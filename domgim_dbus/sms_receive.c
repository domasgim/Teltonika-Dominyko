#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "smsprocess.h"

#include <stdlib.h>
#include "SMS.h"
#include <locale.h>
#include "utf.h"
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h> 

#define DSC_to_msg(DSC) (DSC == 0 ? "Bit7" : (DSC == 1 ? "Bit8" : "UCS2"))

struct termios setup_tty(int serial_port) {
    struct termios tty;

    if(tcgetattr(serial_port, &tty) != 0) {
        fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
        exit(-1);
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 20;    // Wait for up to 1s (10 deciseconds), returning as soon as 20 bits are received.
    tty.c_cc[VMIN] = 128;

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        fprintf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        exit(1);
    }

    return tty;
}

void prepare_program() {
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);

    unsigned char msg0[] = "AT+CSQ\r";
    unsigned char msg1[] = "AT+CNMI=2,1,0,0,0\r";
    unsigned char msg2[] = "AT+CMGF=0\r";
    unsigned char msg4[] = "AT+CMGD=1,4\r";

    printf("Checking signal strength...\n");
    send_gsm_msg(msg0, serial_port);

    printf("Initialising...\n");
    send_gsm_msg(msg1, serial_port);
    send_gsm_msg(msg2, serial_port);
    send_gsm_msg(msg4, serial_port);

    printf("Receiver ready, waiting for messages...\n");

    close(serial_port);
}

void check_message() {
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);

    unsigned char msg3[] = "AT+CMGL=0\r";

    send_gsm_msg(msg3, serial_port);

    close(serial_port);
}

void send_gsm_msg(unsigned char msg[], int *serial_port) {
    char read_buf [65536];
    int signal_flag;

    write(serial_port, msg, strlen(msg));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        printf("Error reading: %s", strerror(errno));
        return;
    }
    //printf("Read %i bytes. Received message: \n%s\n", num_bytes, read_buf);
    process_signal(read_buf, num_bytes);
    process_pdu(read_buf, num_bytes);

    //get_pdu(read_buf, num_bytes);
    return;
}

int process_signal(char msg[], int size) {
    int contains_signal = 0;
    char signal_strength[4];
    int signal_value;

    for (int i = 0; i < size; i++) {
        if (msg[i] == '+' && msg[i + 1] == 'C' && msg[i + 2] == 'S' && 
            msg[i + 3] == 'Q' && msg[i + 4] == ':') {

            //printf("!!!!");
            //contains_signal++;

            strncat(signal_strength, &msg[i + 6], 1);
            strncat(signal_strength, &msg[i + 7], 1);

            signal_value = atoi(signal_strength);

            printf("Signal strength: %d\n\n", signal_value);

            if (signal_value < 3) {
                printf("Error: no signal\n");
                exit(1);

                return 1;
            } else {
                return 0;
            }
        }
    }

    return 0;
}

void send_dbus_signal(char number[], char message[]) {
    SMSProcessGDBUS *proxy;
	GError *error;
	gchar **buf;

	error = NULL;
	proxy = smsprocess_gdbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
			"com.smsprocess", "/com/smsprocess/GDBUS", NULL, &error);

	smsprocess_gdbus_call_send_sms_information_sync(proxy, number, message, buf, NULL, &error);
	//g_print("resp: %s\n", *buf);

	g_object_unref(proxy);
}

void process_pdu(char msg[], int size) {
    int contains_pdu = 0;
    int pdu_msg = 0;
    char pdu[2048];
    char fifo_msg[1024];

    memset(pdu, 0, sizeof pdu);
    //memset(fifo_msg, 0, 1024);

    //printf("FUNKCIJA (endl) \n");
    for (int i = 0; i < size; i++) {
        //printf("%c", msg[i]);

        if (msg[i] == 'C' && msg[i + 1] == 'M' && msg[i + 2] == 'G' && 
            msg[i + 3] == 'L' && msg[i + 4] == ':') {
                //printf("!!!!");
                contains_pdu++;
                }
        
        if (contains_pdu == 1 && msg[i] == '\n' || contains_pdu == 2 && msg[i] == '\n') {

            contains_pdu++;
            //i = i + 3;
            //printf("PDU ZINUTE PRASIDEDA; CONTAINS_PDU = %d ", contains_pdu);

            //strncat(pdu, msg[i], 1);
        }

        if (contains_pdu == 2) {
            strncat(pdu, &msg[i], 1);
        }
    }
    if (contains_pdu > 0) {
        if (pdu[0] == '\n') {
            memmove(pdu, pdu + 1, strlen(pdu));
        }

        printf("\n========================================================================\n");
        printf("PDU = %s\n", pdu);

        struct SMS_Struct s = PDUDecoding(pdu);


        printf("SMSC: %s\n", s.SCA);
        printf("Sender number: %s\n", s.OA);
        printf("Timestamp: %s\n", s.SCTS);
        printf("Message: %s\n", s.UD);
        printf("Encoding： %s\n", DSC_to_msg(s.DCS));

        printf("========================================================================\n\n\n");
        send_dbus_signal(s.OA, s.UD);
    }
    //printf("FUNKCIJOS PABAIGA \n\n");

}

int main() {
    ///*
    prepare_program();
    while (true) {
        check_message();
        sleep(5);
    }
    //*/

    /*
    SMSProcessGDBUS *proxy;
	GError *error;
	gchar **buf;

	error = NULL;
	proxy = smsprocess_gdbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
			"com.smsprocess", "/com/smsprocess/GDBUS", NULL, &error);

	smsprocess_gdbus_call_send_sms_information_sync(proxy, "+37061022009", "Sveiki visi!", buf, NULL, &error);
	//g_print("resp: %s\n", *buf);

	g_object_unref(proxy);
    */
	return 0;
}