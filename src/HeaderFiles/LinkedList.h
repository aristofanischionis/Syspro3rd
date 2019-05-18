#ifndef LINKEDLISTS_HEADER
#define LINKEDLISTS_HEADER

struct Node
{
    int data;
    struct Node *next;
};

typedef struct Node Node;

void deleteList(Node **head_ref);
void deleteNode(Node **head_ref, int key);
void push(Node **head_ref, int new_data);
void printList(Node *n);

#endif