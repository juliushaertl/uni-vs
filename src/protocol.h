/* Verteilte Systeme - Blatt 2
 * Autor: Julius Haertl
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define MSG_TYPE_SUM 0x00
#define MSG_TYPE_COUNT 0x01
#define MSG_TYPE_EXIT 0x02

typedef struct calc_request {
    char request_id;
    char function;
    int argc;
    uint32_t* arguments;
} *calc_request_t;

calc_request_t calc_request_init(int request_id, int function, int argc);
void calc_request_free(calc_request_t r);

#endif

