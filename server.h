#ifndef LIST
#define LIST

typedef struct ClientNode {
    int data;
    struct ClientNode* previous;
    struct ClientNode* next;
    char ip[16];
    char name[31];
    char color[10];
} ClientList;

ClientList *new_node(int sockfd, char* ip) {
    ClientList *np = (ClientList *) malloc( sizeof(ClientList) );
    np->data = sockfd;
    np->previous = NULL;
    np->next = NULL;
    strncpy(np->ip, ip, 16);
    strncpy(np->name, "NULL", 5);
    strncpy(np->color, "NULL", 10);
    return np;
}

#endif // LIST