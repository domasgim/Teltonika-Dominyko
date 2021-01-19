#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pdu.h"

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <ctype.h>
#include <iconv.h>
#include <errno.h>

void usage() {
    printf("Usage: ./a.out phone_number message\n");
    exit(1);
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

void print_command(unsigned char msg[]) {
    for (int i = 0; i < strlen(msg); i++) {
        printf("%c", msg[i]);
    }
    printf("\n");
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

void send_command(char sms[], char number[]) {
    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);
    char * command_cmfg_s ;

    // Converting UTF-8 string to UTF-16BE
    char Output[100];
    size_t insize,out_size;
    memset(Output,0,sizeof(Output));
    int nconv=0;
    char *Inptr;
    char *outptr;  

    iconv_t cd = iconv_open("UTF-16BE","utf-8");
    insize=strlen(sms);
    out_size=3*insize;
    Inptr = sms;
    outptr=(char *)Output;
    nconv=iconv(cd,&Inptr,&insize,&outptr,&out_size);
    int loop_length = strlen(sms) * 2;
    int msg_length = 0;

    if (nconv!=0) {
        fprintf(stderr,"error = %s\n", strerror(errno));
        exit(-1);
        printf("Unable to perform conversion ");
        return 0;
    }
    for (size_t i = 0; i < loop_length; i++) {
        //printf("%3d:%3hhx\n", i, Output[i]);
        msg_length++;

        if (Output[i + 1] == 0 && Output[i + 2] == 0) {
                break;
        }
    }

    // PDU generation
    int alphabet = 2;
    int flash = 0;
    int with_udh = 0;
    char* udh_data = "";
    char* mode = "new";
    int report = 0;
    char pdu[1024] = {};
    int validity = 167; // 1 day
    int system_msg = 0;
    int replace_msg = 0;
    int to_type = 1;

    //make_pdu(argv[1], argv[2], strlen(argv[2]), alphabet, flash, report, with_udh, udh_data, mode, pdu, validity, replace_msg, system_msg, to_type);
    make_pdu(number, Output, msg_length, alphabet, flash, report, with_udh, udh_data, mode, pdu, validity, replace_msg, system_msg, to_type);

    int length = (strlen(pdu) - 2) / 2;
    char TPDU_length[3];

    sprintf(TPDU_length, "%d", length);

    printf("\nPDU: %s\n", pdu);
    printf("PDU Length: %s\n\n", TPDU_length);

    unsigned char msg01[] = "AT\r";
    unsigned char msg11[] = "AT+CMGF=0\r";
    unsigned char msg12[] = "AT+CMGS=";
    unsigned char msg13[1024];
    //unsigned char msg14[] = "\x1A\r";

    unsigned char cmd_end[] = "\r";
    unsigned char msg_end[] = "\x1A\r";

    strncat(msg12, TPDU_length, 4);
    strncat(msg12, cmd_end, 1);

    strncat(msg13, pdu, 512);
    strncat(msg13, msg_end, 5);


    printf("COMMANDS\n");
    printf("%s\n", msg11);
    printf("%s\n", msg12);
    printf("%s\n", msg13);
    printf("\n\n");

    send_gsm_msg(msg01, serial_port);
    send_gsm_msg(msg01, serial_port);
    send_gsm_msg(msg01, serial_port);

    send_gsm_msg(msg11, serial_port);
    send_gsm_msg(msg12, serial_port);
    //send_gsm_msg(msg13, serial_port);
    
    
    // SEND PDU (UNSIGNED VERSION DOESNT WORK)
    char read_buf [256];
    write(serial_port, pdu, strlen(pdu));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        printf("Error reading: %s", strerror(errno));
        return;
    }
    printf("Read %i bytes. Received message: %s", num_bytes, read_buf);

    send_gsm_msg(msg_end, serial_port);

    close(serial_port);
    return; // success
}

int main(int argc, char *argv[]) {
    int fd1; 
  
    // FIFO file path 
    char * fifo_receive = "/tmp/fifo_receive"; 
  
    // Creating the named file(FIFO) 
    // mkfifo(<pathname>,<permission>) 
    mkfifo(fifo_receive, 0666);

    char str1[512]; 

    char msg1[] = "Option-1";
    char msg2[] = "Option-2";
    char msg3[] = "Option-3";
    char msg4[] = "Long live Lithuania";
    while (1) 
    { 
        // First open in read only and read 
        fd1 = open(fifo_receive,O_RDONLY); 
        read(fd1, str1, 512); 
  
        // Print the read string and close 
        printf("SMS received: %s\n", str1); 

        close(fd1); 

        char sms[256];
        char phone[256];
        memset(sms, 0, sizeof sms);
        memset(phone, 0, sizeof phone);

        int flag = 0;

        for (int i = 0; i < strlen(str1); i++) {
            if (str1[i] == ';') {
                flag++;
            }

            if (flag == 0) {
                strncat(sms, &str1[i], 1);
            }

            if (flag == 1 && str1[i] != ';' && str1[i] != '+') {
                strncat(phone, &str1[i], 1);
            }

        }

        printf("Debug | SMS message: %s\n", sms);
        printf("Debug | Phone number: %s\n\n", phone);

        if (sms[0] == '1') {
            printf("Debug | Option 1\n");
            send_command(msg1, phone);
        } else if (sms[0] == '2') {
            printf("Debug | Option 2\n");
            send_command(msg2, phone);
        } else if (sms[0] == '3') {
            printf("Debug | Option 3\n");
            send_command(msg3, phone);
        } else if (sms[0] == '4') {
            printf("Debug | Option 4\n");
            send_command(msg4, phone);
        }
    } 
    return 0;
}
