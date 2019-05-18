#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../HeaderFiles/LinkedList.h"

// printing List
void printList(Node *n)
{
    while (n != NULL)
    {
        printf(" %s %d ", n->IP, n->port);
        n = n->next;
    }
}

void push(Node **head_ref, char* IP, int port)
{
    /* 1. allocate node */
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->IP = malloc(20);


    /* 2. put in the data  */
    strcpy(new_node->IP, IP);
    new_node->port = port;
    /* 3. Make next of new node as head */
    new_node->next = (*head_ref);

    /* 4. move the head to point to the new node */
    (*head_ref) = new_node;
}

/* Given a reference (pointer to pointer) to the head of a list 
   and a key, deletes the first occurrence of key in linked list */
void deleteNode(Node **head_ref, char* IP, int port)
{
    // Store head node
    Node *temp = *head_ref, *prev;

    // If head node itself holds the key to be deleted
    if (temp != NULL && (!strcmp(temp->IP, IP)) && (temp->port == port))
    {
        *head_ref = temp->next; // Changed head
        free(temp);             // free old head
        return;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && ((strcmp(temp->IP, IP)) || (temp->port != port)))
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
    while (temp != NULL)
    {
        prev = temp;
        temp = temp->next;
        free(prev);
    }
}

int exists(Node **head_ref, char *IP, int port)
{
    // Store head node
    Node *temp = *head_ref;

    while (temp != NULL)
    {
        if ((!strcmp(temp->IP, IP)) && (temp->port == port))
        {
            // found it
            return 1;
        }
        temp = temp->next;
    }

    return 0;
}