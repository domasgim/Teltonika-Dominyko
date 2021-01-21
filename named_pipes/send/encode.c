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

void send_gsm_msg(unsigned char msg[], int *serial_port) {
    char read_buf [256];
    write(serial_port, msg, strlen(msg));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        printf("Error reading: %s", strerror(errno)); 
        return;
    }
    process_signal(read_buf, num_bytes);
    //printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
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

void send_sms(char number[], char sms[]) {
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

    printf("PDU: %s\n", pdu);
    printf("PDU Length: %s\n", TPDU_length);

    unsigned char msg11[] = "AT+ F=0\r";
    unsigned char msg12[] = "AT+CMGS=";
    unsigned char msg13[1024];

    unsigned char cmd_end[] = "\r";
    unsigned char msg_end[] = "\x1A\r";

    strncat(msg12, TPDU_length, 4);
    strncat(msg12, cmd_end, 1);

    strncat(msg13, pdu, 512);
    strncat(msg13, msg_end, 5);

    /*
    printf("COMMANDS\n");
    printf("%s\n", msg11);
    printf("%s\n", msg12);
    printf("%s\n", msg13);
    printf("\n\n");
    */

    printf("Sending response...\n");
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
    //printf("Read %i bytes. Received message: %s", num_bytes, read_buf);

    send_gsm_msg(msg_end, serial_port);

    close(serial_port);
    return 0; // success
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

void check_signal() {
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);
    unsigned char msg1[] = "AT+CSQ\r";

    printf("Checking signal strength...\n");
    send_gsm_msg(msg1, serial_port);
}

int main(int argc, char *argv[]) {
    check_signal();
    printf("Program ready, waiting for messages...\n");

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
        printf("========================================================================\n");
        //printf("SMS received: %s\n", str1); 

        close(fd1); 

        char sms[256];
        char phone[256];
        memset(sms, 0, sizeof sms);
        memset(phone, 0, sizeof phone);

        int flag = 0;

        for (int i = 0; i < strlen(str1); i++) {
            //if (str1[i] == ';') {
            if (str1[i] == '\x11') {
                flag++;
            }

            if (flag == 0) {
                strncat(sms, &str1[i], 1);
            }

            if (flag == 1 && str1[i] != '\x11' && str1[i] != '+') {
                strncat(phone, &str1[i], 1);
            }

        }

        printf("SMS message: %s\n", sms);
        printf("Phone number: +%s\n", phone);

        if (sms[0] == '1') {
            printf("Debug | Option 1\n");
            send_sms(phone, msg1);
        } else if (sms[0] == '2') {
            printf("Debug | Option 2\n");
            send_sms(phone, msg2);
        } else if (sms[0] == '3') {
            printf("Debug | Option 3\n");
            send_sms(phone, msg3);
        } else if (sms[0] == '4') {
            printf("Debug | Option 4\n");
            send_sms(phone, msg4);
        }
        printf("========================================================================\n\n\n");
    } 
    return 0; // success
}
