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
 * ToDo:
 * exit Server on 0x02
 * thread exit handling
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
#include <stdarg.h>
#include <pthread.h>
#include <netdb.h>

#include "protocol.h"

#define SERVER_PORT 7890
#define BUFFER_SIZE 6
#define QUEUE_SIZE 5
#define THREAD_NUMBER 4
#define GLOBAL_CMD_EXIT 1

void handle_error_code(char* msg, int err);
void handle_error(char* msg);
int read_request(int listen_s, char* rbuf, int rbuf_size, short* data);


pthread_t threadid[THREAD_NUMBER];
pthread_mutex_t thread_lock;
int global_cmd;

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
        printf("[DEBUG] handle_request sum %d \n", result);
        return result;
    }
    
    /* Return argument count */
    if(request->function==MSG_TYPE_COUNT) {
        uint32_t argc = ntohl((int32_t)request->argc); 
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
    
    /* Allways read first three bytes of protocol */
    while((rw = read(sock_d, header, 3*sizeof(char)))) {

        if(rw < 0 )
            handle_error_code("Error reading protocol header", rw);

        printf("[DEBUG] Received Header: ID %d FUNCTION %02x ARGC %d\n", 
                header[0], header[1], header[2]);

        /* Parse Protocol */
        request = calc_request_init(header[0], header[1], header[2]);

        /* exit server on MSG_TYPE_EXIT */
        if(header[1]==0x02)
            break;

        printf("[DEBUG] Parsing %d arguments\n", header[2]);
        for(i = 0; i<header[2];i++) {
            rw = read(sock_d, &single, 4);
            if(rw < 0)
                handle_error_code("Error reading arguments", rw);

            printf("[DEBUG] ARG %d:%02x\n",i,single);
            request->arguments[i] = ntohl(single);
        }

        
        /* Handle Request */ 
        printf("[DEBUG] Handling Request\n");
        result = handle_request(request);
        
        printf("[DEBUG] Calculated Result %d\n", (uint32_t)result);

        /* Sending Response */
        bzero(buffer, BUFFER_SIZE);
        buffer[0] = request->request_id;
        buffer[1] = request->function;
        memcpy(buffer+2, &result, sizeof(uint32_t));
        printf("Sending response %08x\n", *buffer);
        rw = write(sock_d, buffer, BUFFER_SIZE);
        if(rw < 0) {
            handle_error_code("Error writing to socket", rw);
        }
    }

    close(sock_d);
    printf("Socket closed using 0x02 command\n");
    pthread_exit(0);


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

    int sock_d_server, sock_d_client;
    struct addrinfo flags;
    struct addrinfo *host_info;
    socklen_t addr_size;

    struct sockaddr_storage client;

    pthread_attr_t attr;
    int i;

    global_cmd = 0;

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

    freeaddrinfo(host_info); 

    /* creating listening threads */
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO); 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    listen(sock_d_server, QUEUE_SIZE); 
    addr_size = sizeof(client);
    i = 0;

    while (1) {
        /* check for global server command from threads */
        if(global_cmd) {
        
        }
        if (i == THREAD_NUMBER) {
            i = 0;
        }

        sock_d_client = accept(sock_d_server, (struct sockaddr *) &client, &addr_size);

        if (sock_d_client < 0) {
            handle_error("Error on accept");
        }
        
        pthread_create(&threadid[i++], &attr, &worker_thread, (void *)&sock_d_client);
        sleep(0); /* give threads some cpu time */
    }

    /* cancel all existing threads */


    return 0;



} 

