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
#include "list.h"

node_t* list_create(int value) {
   node_t* start = malloc(sizeof(node_t*)); 
   start->value = value;
   start->next = NULL;
   return start;
}

node_t* list_add(node_t* list, int value) {

    /* create list if it doesn't exist */
    if(list == NULL) {
        list = list_create(value);
        printf("CREATE list %d\n", list->value);
        return list;
    }
    node_t* start = list;

    node_t* item = malloc(sizeof(node_t*));
    item->value = value;
    item->next = NULL;

    while(list->next != NULL) {
        list = list->next;
    }
    list->next = item;
    return start;
}

node_t* list_delete(node_t* list, int value) {
    if(list == NULL)
        return NULL;
    if(list->value == value) {
        node_t* tmp;
        tmp = list->next;
        if(list!=NULL)
            free(list);
        return tmp;
    }
    list->next = list_delete(list->next, value);
    return list;
}

void list_print(node_t* list)  {
    if(list!=NULL) {
    do {
        printf("%d | ", list->value);
        list = list->next;
    } while(list != NULL);
    printf("\n");
    } else {
        printf("Empty list\n");
    }
}

