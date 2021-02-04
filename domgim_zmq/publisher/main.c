#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h> 

#include "src/SMS.h"
#include "src/utf.h"
#include "src/cJSON.h"
#include "src/zhelpers.h"

#define DSC_to_msg(DSC) (DSC == 0 ? "Bit7" : (DSC == 1 ? "Bit8" : "UCS2"))

struct SMS_Struct sms_list[32];
int sms_list_size = 0;
int sms_list_flag = 0;

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

char *create_monitor_with_helpers(char number[], char message[])
{
    char *string = NULL;
    cJSON *resolutions = NULL;
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();

    if (cJSON_AddStringToObject(monitor, "phone_number", number) == NULL) {
        goto end;
    }

    if (cJSON_AddStringToObject(monitor, "message", message) == NULL) {
        goto end;
    }

    string = cJSON_Print(monitor);

    if (string == NULL)
    {
        printf("Failed to print monitor.\n");
    }

end:
    cJSON_Delete(monitor);
    return string;
}

void process_all_pdus(void *socket) {
    char message[2048];
    memset(message, 0, sizeof message);

    for(int i = 0; i < sms_list_size; i++) {

        if (sms_list[i].UDHI == true && sms_list[i].MMS == true) {
            printf("DEBUG: [%d] message: %s length: %d\n", i, sms_list[i].UD, strlen(sms_list[i].UD));
            //strncat(message, &sms_list[i].UD, strlen(sms_list[i].UD - 1));
            strcat(message, sms_list[i].UD);
            //printf("Concated message: %s\n\n", message);

        } else if (sms_list[i].UDHI == true && sms_list[i].MMS == false) {
            printf("DEBUG: [%d] message: %s length: %d\n", i, sms_list[i].UD, strlen(sms_list[i].UD));
            //strncat(message, &sms_list[i].UD, strlen(sms_list[i].UD));
            strcat(message, sms_list[i].UD);
            //printf("Concated message: %s\n", message);

            printf("========================================================================\n");
            printf("SMSC: %s\n", sms_list[i].SCA);
            printf("Sender number: %s\n", sms_list[i].OA);
            printf("Timestamp: %s\n", sms_list[i].SCTS);
            printf("Message: %s\n", message);
            printf("Encoding： %s\n", DSC_to_msg(sms_list[i].DCS));

            char *json_string = create_monitor_with_helpers(sms_list[i].OA, message);
            s_sendmore(socket, "SMS");
            s_send(socket, json_string);

            printf("JSON string sent...\n");
            printf("JSON: %s\n", json_string);
            printf("========================================================================\n\n\n");

            memset(message, 0, sizeof message);
            sleep(1);
        } else {

            printf("========================================================================\n");
            printf("SMSC: %s\n", sms_list[i].SCA);
            printf("Sender number: %s\n", sms_list[i].OA);
            printf("Timestamp: %s\n", sms_list[i].SCTS);
            printf("Message: %s\n", sms_list[i].UD);
            printf("Encoding： %s\n", DSC_to_msg(sms_list[i].DCS));

            char *json_string = create_monitor_with_helpers(sms_list[i].OA, sms_list[i].UD);
            s_sendmore(socket, "SMS");
            s_send(socket, json_string);

            printf("JSON string sent...\n");
            printf("========================================================================\n\n\n");  
            sleep(1); 
        }
    }
}

void process_CMGL(char msg[], int size, void *socket) {
    int contains_header = 0;
    int contains_pdu = 0;
    struct SMS_Struct sms;

    char pdu[2048];

    memset(pdu, 0, sizeof pdu);

    for (int i = 0; i < size; i++) {
        if (msg[i] == 'C' && msg[i + 1] == 'M' && msg[i + 2] == 'G' && 
            msg[i + 3] == 'L' && msg[i + 4] == ':') {
                contains_header = 1;
                }
        
        if (contains_header == 1 && msg[i] == '\n') {
            contains_header = 0;
            contains_pdu = 1;
            i++;
        }

        if (contains_pdu == 1) {
            strncat(pdu, &msg[i], 1);
        }

        if (contains_pdu == 1 && msg[i] == '\n') {
            contains_pdu = 0;
            sms = PDUDecoding(pdu);

            sms_list[sms_list_size] = sms;
            sms_list_size++;
            sms_list_flag++;

            memset(pdu, 0, sizeof pdu);
            i++;
        }
    }
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

void send_gsm_msg(unsigned char msg[], int *serial_port, void *socket) {
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
    process_CMGL(read_buf, num_bytes, socket);

    //get_pdu(read_buf, num_bytes);
    return;
}

void prepare_program() {
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);

    unsigned char msg0[] = "AT+CSQ\r";
    unsigned char msg1[] = "AT+CNMI=2,1,0,0,0\r";
    unsigned char msg2[] = "AT+CMGF=0\r";
    unsigned char msg4[] = "AT+CMGD=1,4\r";

    printf("Checking signal strength...\n");
    send_gsm_msg(msg0, serial_port, NULL);

    printf("Initialising...\n");
    send_gsm_msg(msg1, serial_port, NULL);
    send_gsm_msg(msg2, serial_port, NULL);
    send_gsm_msg(msg4, serial_port, NULL);

    printf("Receiver ready, waiting for messages...\n");

    close(serial_port);
}

int main() {
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket (context, ZMQ_PUB);
    zmq_bind (publisher, "tcp://*:5563");

    prepare_program();

    while (true) {
        int serial_port = open("/dev/ttyUSB2", O_RDWR);
        unsigned char msg3[] = "AT+CMGL=0\r";

        setup_tty(serial_port);
        send_gsm_msg(msg3, serial_port, publisher);
        close(serial_port);

        if (sms_list[sms_list_size - 1].MMS == false) {
            process_all_pdus(publisher);
            sms_list_size = 0;
            memset(sms_list, 0, sizeof(sms_list));
        }
        sleep(5);
    }

    return 0;
}
