#ifndef ___LINKED__LIST__H__
#define ___LINKED__LIST__H__
/* MODIFIED LINKED LIST IMPLEMENTATION FROM LECTURE SLIDES */


/**
 * @brief Node type for linked list
 * 
 */
struct node {
 int PID; /* the data to be stored */
 char* command;
 struct node *next; /* pointer to next list element */
};


/**
 * @brief List type for administration of linked list
 */
struct list {
 struct node *head; /* first element */
 struct node **tail; /* 'next' pointer in last element */
};


/**
 * @brief add a new node to the end of the list
 * 
 * @param list 
 * @param item 
 */
//void add_node (struct list *list, struct node *item);
void add_node (struct list *list, int PID, char* cmd);


/**
 * @brief Create a list object
 * 
 * @return struct list* 
 */
struct list* create_list();


/**
 * @brief Prints the index and PID of every item in the list, including head
 * 
 * @param list 
 */
void print_list (struct list *list);
#endif