#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../HeaderFiles/LinkedList.h"
#include "../HeaderFiles/Common.h"
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


void listToString(Node *head_ref, char **result, char *IP, int port)
{
    int nodes = 0;
    Node *temp = head_ref;
    nodes = countNodes(head_ref);
    char *initStr, *retrIpPort;
    char *str1, *str2;
    initStr = malloc(30);
    retrIpPort = malloc(60);
    str1 = malloc(BUFSIZ);
    str2 = malloc(60);
    initStr[0] = '\0';
    nodes--;
    // minus the node that is us from the list
    sprintf(initStr, "CLIENT_LIST %d ", nodes);
    strcpy(*result, initStr);
    
    while (temp != NULL)
    {
        if ((!strcmp(temp->IP, IP)) && (port == temp->port))
        {
            // then it is the same client don't include him in the list
            temp = temp->next;
            continue;
        }
        sprintf(retrIpPort, "< %s , %d > ", temp->IP, temp->port);
        strcpy(str1, *result);
        strcpy(str2, retrIpPort);
        appendString(result, str1, str2);
        temp = temp->next;
    }

    free(initStr);
    free(retrIpPort);
    free(str1);
    free(str2);
}