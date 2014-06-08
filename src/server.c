/* Verteilte Systeme - Blatt 2
 * Autor: Julius Härtl
 *
 * Server Application
 *
 * Simple implementation of a tcp server program that can run the following
 * predefined functions:
 * 
 *  - Sum all given arguments
 *  - Count the given arguments
 *  - Shutdown the server
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <pthread.h>

#include "protocol.h"
#include "list.h"

#define BUFFER_SIZE 6
#define QUEUE_SIZE 5

void handle_error_code(char* msg, int err);
void handle_error(char* msg);
void exit_server();
int read_request(int listen_s, char* rbuf, int rbuf_size, short* data);

/* global variables */
pthread_mutex_t socket_list_lock;
node_t* socket_list;
int sock_d_server, sock_d_client;

/* make calculations based on calc_request_t
 * return -1 on error
 * could be improved by returning pointer (error handling)
 */
uint32_t handle_request(calc_request_t request) {

    uint32_t result = 0;
    int i = 0;

    /* Sum all arguments */
    if(request->function==MSG_TYPE_SUM) {
        for(i=0;i<request->argc;i++) {
            result += request->arguments[i];
        }
        return result;
    }
    
    /* Return argument count */
    if(request->function==MSG_TYPE_COUNT) {
        uint32_t argc = (int32_t)request->argc; 
        printf("[DEBUG] handle_request count %d \n", argc);
        return argc;
    }

    return -1;
}

/* Thread Handler for Socket Connection */
void* worker_thread(void* arg) {
   
    /* socket */
    int i;
    int sock_d, rw; 
    sock_d = *(int*)arg;
    unsigned char cmd;

    /* request */
    char header[3];
    calc_request_t request;
    uint32_t single;
    uint32_t result;

    /* response */
    char* buffer;

    /* initialize buffer with 0 */
    buffer = malloc(BUFFER_SIZE);
    bzero(buffer, BUFFER_SIZE);
    
    pthread_mutex_lock(&socket_list_lock);
    socket_list = list_add(socket_list, sock_d);
    printf("Added Socket to list:\n");
    list_print(socket_list);
    pthread_mutex_unlock(&socket_list_lock);

    printf("[DEBUG] Worker Thread started\n");
    /* Allways read first three bytes of protocol */
    while((rw = read(sock_d, header, 3*sizeof(char)))) {

        if(rw < 0 )
            handle_error_code("Error reading protocol header", rw);

        printf("[DEBUG] Received Header: ID %d FUNCTION %02x ARGC %d\n", 
                header[0], header[1], header[2]);

        /* Parse Protocol */
        request = calc_request_init(header[0], header[1], header[2]);

        cmd = header[1];

        /* exit server on MSG_TYPE_EXIT */
        if(header[1]==MSG_TYPE_EXIT)
            break;

        printf("[DEBUG] Parsing %d arguments\n", header[2]);
        for(i = 0; i<header[2];i++) {
            rw = read(sock_d, &single, 4);
            if(rw < 0)
                handle_error_code("Error reading arguments", rw);
            request->arguments[i] = ntohl(single);
            printf("[DEBUG] ARG %d = %u\n",i,request->arguments[i]);
        }

        
        /* Handle Request */ 
        printf("[DEBUG] Handling Request\n");
        result = handle_request(request);
        printf("[DEBUG] Calculated Result %u\n", (uint32_t)result);

        /* Sending Response */
        bzero(buffer, BUFFER_SIZE);
        buffer[0] = request->request_id;
        buffer[1] = request->function;
        result = htonl(result);
        memcpy(buffer+2, &result, sizeof(uint32_t));
        printf("[DEBUG] Sending response %08x\n", *(buffer+2));
        rw = write(sock_d, buffer, BUFFER_SIZE);
        if(rw < 0) {
            handle_error_code("Error writing to socket", rw);
        }
    }
    
    free(buffer);
    if(cmd==0x02) {
        exit_server();
    }
    printf("[DEBUG] Worker Thread ended\n");
    close(sock_d);

    pthread_mutex_lock(&socket_list_lock);
    socket_list = list_delete(socket_list, sock_d);
    pthread_mutex_unlock(&socket_list_lock);

    pthread_exit(0);

}
void exit_server() {

    /* closing sockets */
    pthread_mutex_lock(&socket_list_lock);
    while(socket_list != NULL) {
        printf("Closing socket %d \n", socket_list->value);
        close(socket_list->value);
        socket_list = list_delete(socket_list,socket_list->value);
    }
    pthread_mutex_unlock(&socket_list_lock);

    close(sock_d_server);

    exit(0);

}
/* error handling functions */
void handle_error_code(char* msg, int err) {

    if(err==0) {
        fprintf(stderr, "[ERROR] %s\n", msg);
        err=-1;
    } else {
        fprintf(stderr, "[ERROR] %s with error code %d\n", msg, err);
    }
    perror(NULL);
    exit(err);

}

void handle_error(char* msg) {

    handle_error_code(msg, 0);

}


int main(int argc, char** argv) {

    /* ignore unused variable */
    (void)argc;

    struct addrinfo flags;
    struct addrinfo *host_info;
    socklen_t addr_size;
    pthread_t threadid;

    struct sockaddr_storage client;

    pthread_attr_t attr;

    pthread_mutex_init(&socket_list_lock, NULL);

    if (argc < 2) {
        handle_error("Error: no port provided\n");
    }

    /* configure socket */
    memset(&flags, 0, sizeof(flags));
    flags.ai_family = AF_UNSPEC; /* IPv4 & IPv6 */
    flags.ai_socktype = SOCK_STREAM; /* TCP */
    flags.ai_flags = AI_PASSIVE; /* Set address */

    if (getaddrinfo(NULL, argv[1], &flags, &host_info) < 0) {
        handle_error("Couldn't read host info for socket start");
    }

    sock_d_server = socket(host_info->ai_family,
            host_info->ai_socktype,
            host_info->ai_protocol);

    if (sock_d_server < 0) {
        handle_error("Error opening socket");
    }

    if (bind(sock_d_server, host_info->ai_addr, host_info->ai_addrlen) < 0) {
        handle_error("Error on binding");
    }
    printf("[DEBUG] Bind to Port %s \n", argv[1]);

    freeaddrinfo(host_info); 

    /* creating listening threads */
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO); 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    listen(sock_d_server, QUEUE_SIZE); 
    addr_size = sizeof(client);

    while (1) {

        sock_d_client = accept(sock_d_server, (struct sockaddr *) &client, &addr_size);

        if (sock_d_client < 0) {
            handle_error("Error on accept");
        }
            
        pthread_create(&threadid, &attr, &worker_thread, (void *)&sock_d_client);
        sleep(0); 
    }

    return 0;



} 

