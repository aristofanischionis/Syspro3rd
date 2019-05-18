#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../HeaderFiles/LinkedList.h"

// printing List
void printList(Node *n)
{
    while (n != NULL)
    {
        printf(" %d ", n->data);
        n = n->next;
    }
}

void push(Node **head_ref, int new_data)
{
    /* 1. allocate node */
    Node *new_node = (Node *)malloc(sizeof(Node));

    /* 2. put in the data  */
    new_node->data = new_data;

    /* 3. Make next of new node as head */
    new_node->next = (*head_ref);

    /* 4. move the head to point to the new node */
    (*head_ref) = new_node;
}

/* Given a reference (pointer to pointer) to the head of a list 
   and a key, deletes the first occurrence of key in linked list */
void deleteNode(Node **head_ref, int key)
{
    // Store head node
    Node *temp = *head_ref, *prev;

    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->data == key)
    {
        *head_ref = temp->next; // Changed head
        free(temp);             // free old head
        return;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->data != key)
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL)
        return;

    // Unlink the node from linked list
    prev->next = temp->next;

    free(temp); // Free memory
}

void deleteList(Node **head_ref)
{
    Node *temp = *head_ref, *prev;
    while(temp != NULL){
        prev = temp;
        temp = temp->next;
        free(prev);
    }
}