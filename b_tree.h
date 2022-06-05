#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define min_degree 5
#define max_children min_degree * 2
#define max_keys max_children - 1
#define min_keys min_degree - 1

struct BTreeNode
{
    bool leaf;
    int key[max_keys + 1];
    int count_key;
    struct BTreeNode *child[max_children + 1];
    int count_child;
};
int delete_value(int val, struct BTreeNode *node);
struct BTreeNode *root;