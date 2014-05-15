#include "node.h"

struct node* node_alloc(sds ip, int port, int weight)
{
    struct node *n = malloc(sizeof(struct node));

    n->ip = ip;
    n->port = port;
    n->weight = weight;
    n->next = NULL;

    return n;
}

void node_free(struct node* n)
{
    if(!n)
        return;

    if(n->ip)
        sdsfree(n->ip);

    free(n);
}

void node_free_all(struct node *n)
{
    struct node *nn;
    while(n)
    {
        nn = n->next;
        node_free(n);
        n = nn;
    }
}

int node_count(struct node *n)
{
    int cnt = 0;
    struct node *nn = n;
    while(nn)
    {
        cnt++;
        nn = nn->next;
    }

    return cnt;
}
