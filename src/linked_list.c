#include <stdlib.h>
#include "linked_list.h"
#include <stdio.h>
/* MODIFIED LINKED LIST IMPLEMENTATION FROM LECTURE SLIDES */

void add_node (struct list *list, int PID, char* cmd) {
    struct node* item = malloc(sizeof(struct node));
    item->command = cmd;
    item->PID = PID;
    item->next = NULL;
    *list->tail = item;
    list->tail = &item->next;
}

struct list* create_list() {
    struct list* l;
    struct node *item = calloc(1, sizeof(struct node));
    l = calloc(1, sizeof(struct list));
    l->head = item;
    l->tail = &item->next;
    return l;
}

void print_list (struct list *list) {
    struct node *n = list->head->next;
    while(n!=NULL) {
        printf("[%d] %s", n->PID, n->command);
        n = n->next;
    }
}
