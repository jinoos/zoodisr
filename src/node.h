#ifndef _NODE_H_
#define _NODE_H_

#include "zoodisr.h"

struct node
{
    sds ip;
    int port;
    int weight;

    struct node *next;
};

struct node* node_alloc(sds ip, int port, int weight);
void node_free(struct node* n);
void node_free_all(struct node *n);
int node_count(struct node *n);

#endif // _NODE_H_
