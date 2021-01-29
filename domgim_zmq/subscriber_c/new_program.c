// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <stdlib.h>
#include <ctype.h>

void usage() {
    printf("Usage: ./a.out phone_number message\n");
    exit(1);
}

int digits_only(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0) return 1;
    }

    return 0;
}

void send_gsm_msg(unsigned char msg[], int *serial_port) {
    char read_buf [256];
    write(serial_port, msg, strlen(msg));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        printf("Error reading: %s", strerror(errno));
        return;
    }
    printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
    return;
}

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

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as 20 bits are received.
    tty.c_cc[VMIN] = 20;

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

int main(int argc, char *argv[]) {



    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);

    char * command_phone ;
    char * command_msg;

    if((command_phone = malloc(strlen(argv[1] + 1 + 13))) != NULL){
            command_phone[0] = '\0';   // ensures the memory is an empty string
            strcat(command_phone, "AT+CMGS=\"");
            strcat(command_phone, argv[1]);
            strcat(command_phone, "\"\r");
    } else {
            printf("malloc failed!\n");
            return 1;
    }

    if((command_msg = malloc(strlen(argv[2] + 1 + 1))) != NULL){
            command_msg[0] = '\0';   // ensures the memory is an empty string
            strcat(command_msg, argv[2]);
            strcat(command_msg, "\r");
    } else {
            printf("malloc failed!\n");
            return 1;
    }

    unsigned char msg1[] = { 'A', 'T', 'E', '1', '\r' };
    unsigned char msg2[] = { 'A', 'T', '+', 'C', 'M', 'G', 'F', '=', '1', '\r' };
    unsigned char msg3[strlen(argv[1]) + 11];
    unsigned char msg4[strlen(argv[2])];
    unsigned char msg5[] = { '\x1A', '\r' };

    for(int i = 0; i < sizeof(msg3); i++) {
        msg3[i] = command_phone[i];
    }

    for(int i = 0; i < sizeof(msg4); i++) {
        msg4[i] = command_msg[i];
    }

    //send_gsm_msg(msg1, serial_port);
    send_gsm_msg(msg2, serial_port);
    send_gsm_msg(msg3, serial_port);
    send_gsm_msg(msg4, serial_port);
    send_gsm_msg(msg5, serial_port);

    close(serial_port);
    return 0; // success
}