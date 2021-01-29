#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <zmq.h>

int main() {
    void* context = zmq_ctx_new();
    printf("Client Starting....\n");
    void* request = zmq_socket(context, ZMQ_REQ);  
    zmq_connect(request, "tcp://localhost:4040");

    for(;;) {
        zmq_msg_t reply;    
        zmq_msg_init(&reply);    
        zmq_msg_recv(&reply, request, 0);    
        printf("Received: %s\n", (char *) zmq_msg_data(&reply));    
        zmq_msg_close(&reply);

        uint64_t more_part;    
        size_t more_size = sizeof(more_part);    
        zmq_getsockopt(request, ZMQ_RCVMORE, &more_part, &more_size);

        if (!more_part)      
            break;
    }

    zmq_close(request);  
    zmq_ctx_destroy(context);

    return 0;
}