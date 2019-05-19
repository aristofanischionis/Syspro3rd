#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../HeaderFiles/LinkedList.h"

// printing List
void printList(Node *n)
{
    int i = 1;
    while (n != NULL)
    {
        printf("Client %d is-> %s %d \n", i, n->IP, n->port);
        n = n->next;
        i++;
    }
}

int countNodes(Node *n)
{
    int i = 0;
    while (n != NULL)
    {
        n = n->next;
        i++;
    }
    return i;
}
// pushing a new unique object in my list
void push(Node **head_ref, char *IP, int port)
{

    if (exists(head_ref, IP, port))
    {
        printf("This IP and Port already exist\n");
        return;
    }
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
int deleteNode(Node **head_ref, char *IP, int port)
{
    // Store head node
    Node *temp = *head_ref, *prev;

    // If head node itself holds the key to be deleted
    if (temp != NULL && (!strcmp(temp->IP, IP)) && (temp->port == port))
    {
        *head_ref = temp->next; // Changed head
        free(temp);             // free old head
        return 1;
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
        return 0;

    // Unlink the node from linked list
    prev->next = temp->next;

    free(temp); // Free memory
    return 1;
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
    Node *temp = *head_ref, *temp1 = *head_ref;

    while (temp != NULL)
    {
        if ((!strcmp(temp->IP, IP)) && (temp->port == port))
        {
            // found it
            // reinitialize head
            head_ref = &temp1;
            return 1;
        }
        temp = temp->next;
    }
    // reinitialize head
    head_ref = &temp1;
    return 0;
}

void appendString(char *new_str, char *str1, char *str2)
{
    if ((new_str = malloc(strlen(str1) + strlen(str2) + 1)) != NULL)
    {
        new_str[0] = '\0'; // ensures the memory is an empty string
        strcat(new_str, str1);
        strcat(new_str, str2);
    }
    else
    {
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }
}

void listToString(Node **head_ref, char *result)
{
    int nodes = 0;
    Node *temp = *head_ref;
    nodes = countNodes(*head_ref);
    char *initStr, *retrIpPort, *res;
    res = malloc(30);
    initStr = malloc(30);
    retrIpPort = malloc(30);
    initStr[0] = '\0';
    sprintf(initStr, "CLIENT_LIST %d ", nodes);
    strcpy(res, initStr);

    while (temp != NULL)
    {
        sprintf(retrIpPort, "< %s , %d > ", temp->IP, temp->port);
        char *str1, *str2;
        str1 = malloc(30);
        str2 = malloc(30);
        strcpy(str1, res);
        strcpy(str2, retrIpPort);
        free(res);
        appendString(res, str1, str2);
        temp = temp->next;
    }

    printf(":::::::This is the list to string function::::::::::\n");
    printf("My final string is : %s\n", res);

    strcpy(result, res);
}