//  Pubsub envelope publisher
//  Note that the zhelpers.h file also provides s_sendmore

#include "zhelpers.h"
#include <unistd.h>
#include "cJSON.h"
#include <stdio.h>

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

int main (void)
{
    //  Prepare our context and publisher
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket (context, ZMQ_PUB);
    char *message = create_monitor_with_helpers("+37061022009", "Labas visi draugužiai!");

    zmq_bind (publisher, "tcp://*:5563");


    while (1) {
        //  Write two messages, each with an envelope and content
        /*
        s_sendmore (publisher, "A");
        s_send (publisher, "We don't want to see this");
        s_sendmore (publisher, "B");
        s_send (publisher, "We would like to see this");
        */

        s_sendmore (publisher, "A");
        s_send (publisher, "We don't want to see this");
        s_sendmore (publisher, "B");
        s_send (publisher, message);

        /*
        s_sendmore (publisher, "phone");
        s_send (publisher, "+37061022009");
        s_sendmore (publisher, "message");
        s_send (publisher, "Ačiū už vakarienę!");
        */
        
        sleep (5);
    }
    //  We never get here, but clean up anyhow
    zmq_close (publisher);
    zmq_ctx_destroy (context);
    return 0;
}
