//  Pubsub envelope subscriber

#include "zhelpers.h"
#include "cJSON.h"
#include <errno.h>
#include <stdio.h>

void read_monitor_data(const char * const monitor) {
    const cJSON *phone_number;
    const cJSON *message;
    cJSON *monitor_json = cJSON_Parse(monitor);
    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        goto end;
    }

    phone_number = cJSON_GetObjectItemCaseSensitive(monitor_json, "phone_number");
    message = cJSON_GetObjectItemCaseSensitive(monitor_json, "message");

    printf("Phone number: %s\n", phone_number->valuestring);
    printf("Message contents: %s\n", message->valuestring);

    
    end:
        cJSON_Delete(monitor_json);
}

int main (void)
{
    //  Prepare our context and subscriber
    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    zmq_connect (subscriber, "tcp://localhost:5563");
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, "B", 1);

    /*
    void *subscriber_phone = zmq_socket (context, ZMQ_SUB);
    zmq_connect (subscriber_phone, "tcp://localhost:5563");
    zmq_setsockopt (subscriber_phone, ZMQ_SUBSCRIBE, "phone", 1);

    void *subscriber_message = zmq_socket (context, ZMQ_SUB);
    zmq_connect (subscriber_message, "tcp://localhost:5563");
    zmq_setsockopt (subscriber_message, ZMQ_SUBSCRIBE, "message", 1);
    */

    while (1) {
        //  Read envelope with address
        char *address = s_recv (subscriber);
        //  Read message contents
        char *contents = s_recv (subscriber);
        read_monitor_data(contents);
        //printf ("[%s] %s %s\n", address, contents);
        free (address);
        free (contents);

        /*
        char *address_phone = s_recv (subscriber_phone);
        char *phone_number = s_recv (subscriber_phone);

        char *address_message = s_recv (subscriber_message);
        char *message_contents = s_recv (subscriber_message);

        printf("==========\n");
        printf("[%s] %s\n", address_phone, phone_number);
        printf("[%s] %s\n", address_message, message_contents);
        printf("==========\n\n");

        free (address_phone);
        free (address_message);
        free (phone_number);
        free (message_contents);
        */
    }
    //  We never get here, but clean up anyhow
    //zmq_close (subscriber);
    //zmq_ctx_destroy (context);
    return 0;
}