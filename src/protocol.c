/* Verteilte Systeme - Blatt 2
 * Autor: Julius Haertl
 */

#include <stdlib.h>
#include "protocol.h"

/* allocate memory for request */
calc_request_t calc_request_init(int request_id, int function, int argc) {

    calc_request_t r = malloc(sizeof(calc_request_t));
    r->request_id = request_id;
    r->function = function;

    if(function==MSG_TYPE_SUM || function==MSG_TYPE_COUNT) {
        r->arguments = malloc(sizeof(uint32_t*)*argc);
        r->argc = argc;
    }

    return r;

}

void calc_request_free(calc_request_t r) {
    if(r->function != 0x02) {
        free(r->arguments);
    }
    free(r);
}


