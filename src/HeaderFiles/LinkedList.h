#ifndef LINKEDLISTS_HEADER
#define LINKEDLISTS_HEADER

struct Node
{
    char* IP;
    int port;
    struct Node *next;
};

typedef struct Node Node;

void deleteList(Node **head_ref);
int deleteNode(Node **head_ref, char *IP, int port);
void appendString(char **new_str, char *str1, char *str2);
void push(Node **head_ref, char *IP, int port);
void printList(Node *n);
int countNodes(Node *n);
void listToString(Node *head_ref, char **result, char* IP, int port);
int exists(Node **head_ref, char *IP, int port);

#endif