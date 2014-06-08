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

typedef struct node {
    int value;
    struct node *next;
} node_t;

node_t* list_create(int value);

node_t* list_add(node_t* list, int value);

node_t* list_delete(node_t* list, int value);

void list_print(node_t* list);

