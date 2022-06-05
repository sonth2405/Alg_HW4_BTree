#include "b_tree.h"

int search(struct BTreeNode *node, int val)
{
    if (!node)
    {
        return 0;
    }
    struct BTreeNode *level = node;
    while (true)
    {
        int pos;
        for (pos = 0; pos < level->count_key; pos++)
        {
            if (val == level->key[pos])
            {
                return 1;
            }
            else if (val < level->key[pos])
            {
                break;
            }
        }
        if (level->leaf)
            break;
        level = level->child[pos];
    }
    return 0;
}

struct BTreeNode *create_node(int val)
{
    struct BTreeNode *new_node;
    new_node = (struct BTreeNode *)malloc(sizeof(struct BTreeNode));
    new_node->leaf = false;
    new_node->key[0] = val;
    new_node->count_key = 1;
    new_node->count_child = 0;
    return new_node;
}

struct BTreeNode *split(int pos, struct BTreeNode *node, struct BTreeNode *parent)
{
    int mid;
    mid = node->count_key / 2;
    struct BTreeNode *right;

    right = (struct BTreeNode *)malloc(sizeof(struct BTreeNode));
    right->leaf = node->leaf;
    right->count_key = 0;
    right->count_child = 0;

    int num = node->count_key;
    for (int i = mid + 1; i < num; i++)
    {
        right->key[i - (mid + 1)] = node->key[i];
        right->count_key++;
        node->count_key--;
    }

    if (!node->leaf)
    {
        num = node->count_child;
        for (int i = mid + 1; i < num; i++)
        {
            right->child[i - (mid + 1)] = node->child[i];
            right->count_child++;
            node->count_child--;
        }
    }

    if (node == root)
    {
        struct BTreeNode *new_parent_node;
        new_parent_node = create_node(node->key[mid]);
        node->count_key--;
        new_parent_node->child[0] = node;
        new_parent_node->child[1] = right;

        new_parent_node->count_child = 2;
        return new_parent_node;
    }
    else
    {
        for (int i = parent->count_key; i > pos; i--)
        {
            parent->key[i] = parent->key[i - 1];
            parent->child[i + 1] = parent->child[i];
        }

        parent->key[pos] = node->key[mid];
        parent->count_key++;
        node->count_key--;
        parent->child[pos + 1] = right;
        parent->count_child += 1;
    }
    return node;
}

struct BTreeNode *insert_node(int parent_pos, int val, struct BTreeNode *node, struct BTreeNode *parent)
{
    int pos;
    for (pos = 0; pos < node->count_key; pos++)
    {
        if (val == node->key[pos])
        {
            return node;
        }
        else if (val < node->key[pos])
        {
            break;
        }
    }
    if (!node->leaf)
    {
        node->child[pos] = insert_node(pos, val, node->child[pos], node);
        if (node->count_key == max_keys + 1)
        {
            node = split(parent_pos, node, parent);
        }
    }
    else
    {
        for (int i = node->count_key; i > pos; i--)
        {
            node->key[i] = node->key[i - 1];
            node->child[i + 1] = node->child[i];
        }

        node->key[pos] = val;
        node->count_key++;
        if (node->count_key == max_keys + 1)
        {
            node = split(parent_pos, node, parent);
        }
    }
    return node;
}

void insert(int val)
{
    if (!root)
    {
        root = create_node(val);
        root->leaf = true;
        return;
    }
    else
    {
        root = insert_node(0, val, root, root);
    }
}

void merge(struct BTreeNode *parent_node, int pos, int merge_pos)
{

    int merge_idx = parent_node->child[merge_pos]->count_key;
    parent_node->child[merge_pos]->key[merge_idx] = parent_node->key[merge_pos];
    parent_node->child[merge_pos]->count_key++;

    for (int i = 0; i < parent_node->child[pos]->count_key; i++)
    {
        parent_node->child[merge_pos]->key[merge_idx + 1 + i] = parent_node->child[pos]->key[i];
    }

    int merge_child_index = parent_node->child[merge_pos]->count_child;
    for (int i = 0; i < parent_node->child[pos]->count_child; i++)
    {
        parent_node->child[merge_pos]->child[merge_child_index + i] = parent_node->child[pos]->child[i];
        parent_node->child[merge_pos]->count_child++;
    }

    free(parent_node->child[pos]);

    for (int i = merge_pos; i < (parent_node->count_key) - 1; i++)
    {
        parent_node->key[i] = parent_node->key[i + 1];
    }
    parent_node->count_key--;

    for (int i = merge_pos + 1; i < (parent_node->count_child) - 1; i++)
    {
        parent_node->child[i] = parent_node->child[i + 1];
    }
    parent_node->count_child--;
}

void borrow_from_left(struct BTreeNode *parent_node, int current)
{
    int index = 0;

    for (int i = 0; i < parent_node->child[current]->count_key; i++)
    {
        parent_node->child[current]->key[i + 1] = parent_node->child[current]->key[i];
    }
    parent_node->child[current]->key[index] = parent_node->key[current - 1];
    parent_node->child[current]->count_key++;

    int sibling_index_top = (parent_node->child[current - 1]->count_key) - 1;
    parent_node->key[current - 1] = parent_node->child[current - 1]->key[sibling_index_top];
    parent_node->child[current - 1]->count_key--;

    if (parent_node->child[current - 1]->count_child > 0)
    {
        int child_index = (parent_node->child[current - 1]->count_child) - 1;

        for (int i = parent_node->child[current]->count_child; i > 0; i--)
        {
            parent_node->child[current]->child[i] = parent_node->child[current]->child[i - 1];
        }

        parent_node->child[current]->child[0] = parent_node->child[current - 1]->child[child_index];
        parent_node->child[current]->count_child++;

        parent_node->child[current - 1]->count_child--;
    }
}

void borrow_from_right(struct BTreeNode *parent_node, int current)
{
    int index = parent_node->child[current]->count_key;
    parent_node->child[current]->key[index] = parent_node->key[current];
    parent_node->child[current]->count_key++;

    int sibling_index_top = 0;
    parent_node->key[current] = parent_node->child[current + 1]->key[sibling_index_top];

    for (int i = 0; i < (parent_node->child[current + 1]->count_key) - 1; i++)
    {
        parent_node->child[current + 1]->key[i] = parent_node->child[current + 1]->key[i + 1];
    }
    parent_node->child[current + 1]->count_key--;

    int sibling_index = 0;

    if (parent_node->child[current + 1]->count_child > 0)
    {
        int child_index = parent_node->child[current]->count_child;
        parent_node->child[current]->child[child_index] = parent_node->child[current + 1]->child[sibling_index];
        parent_node->child[current]->count_child++;

        for (int i = 0; i < parent_node->child[current + 1]->count_child - 1; i++)
        {
            parent_node->child[current + 1]->child[i] = parent_node->child[current + 1]->child[i + 1];
        }
        parent_node->child[current + 1]->count_child--;
    }
}

void balance(struct BTreeNode *node, int child_pos)
{
    if (child_pos == 0)
    {
        if (node->child[child_pos + 1]->count_key > min_keys)
        {
            borrow_from_right(node, child_pos);
        }
        else
        {
            merge(node, child_pos + 1, child_pos);
        }
        return;
    }

    else if (child_pos == (node->count_key))
    {
        if (node->child[child_pos - 1]->count_key > min_keys)
        {
            borrow_from_left(node, child_pos);
        }
        else
        {
            merge(node, child_pos, child_pos - 1);
        }
        return;
    }
    else
    {
        if (node->child[child_pos - 1]->count_key > min_keys)
        {
            borrow_from_left(node, child_pos);
        }
        else if (node->child[child_pos + 1]->count_key > min_keys)
        {
            borrow_from_right(node, child_pos);
        }
        else
        {
            merge(node, child_pos, child_pos - 1);
        }
        return;
    }
}

int merge_child(struct BTreeNode *parent_node, int current)
{
    int merge_index = parent_node->child[current]->count_key;

    int val_parent_node = parent_node->key[current];
    parent_node->child[current]->key[merge_index] = parent_node->key[current];
    parent_node->child[current]->count_key++;

    for (int i = 0; i < parent_node->child[current + 1]->count_key; i++)
    {
        parent_node->child[current]->key[merge_index + 1 + i] = parent_node->child[current + 1]->key[i];
        parent_node->child[current]->count_key++;
    }

    for (int i = 0; i < parent_node->child[current + 1]->count_child; i++)
    {
        parent_node->child[current]->child[merge_index + 1 + i] = parent_node->child[current + 1]->child[i];
        parent_node->child[current]->count_child++;
    }

    for (int i = current; i < parent_node->count_key; i++)
    {
        parent_node->key[i] = parent_node->key[i + 1];
        parent_node->count_key--;
    }
    for (int i = current + 1; i < parent_node->count_child; i++)
    {
        parent_node->child[i] = parent_node->child[i + 1];
        parent_node->count_child--;
    }
    return val_parent_node;
}

int find_predecessor(struct BTreeNode *current_node)
{
    int predecessor;
    if (current_node->leaf)
    {
        return current_node->key[current_node->count_key - 1];
    }
    return find_predecessor(current_node->child[(current_node->count_child) - 1]);
}

int override_predecessor(struct BTreeNode *parent_node, int pos_search)
{
    int predecessor = find_predecessor(parent_node->child[pos_search]);
    parent_node->key[pos_search] = predecessor;
    return predecessor;
}

int find_successor(struct BTreeNode *current_node)
{
    int successor;
    if (current_node->leaf)
    {
        return current_node->key[0];
    }
    return find_successor(current_node->child[0]);
}

int override_successor(struct BTreeNode *parent_node, int pos_search)
{
    int successor = find_successor(parent_node->child[pos_search + 1]);
    parent_node->key[pos_search] = successor;
    return successor;
}

void delete_inner(struct BTreeNode *current_node, int current)
{
    int cessor = 0;
    int deleted_for_merge = 0;

    if (current_node->child[current]->count_key >= current_node->child[current + 1]->count_key)
    {
        if (current_node->child[current]->count_key > min_keys)
        {
            cessor = override_predecessor(current_node, current);
            delete_value(cessor, current_node->child[current]);
        }
        else
        {
            deleted_for_merge = merge_child(current_node, current);
            delete_value(deleted_for_merge, current_node->child[current]);
        }
    }
    else
    {
        if (current_node->child[current + 1]->count_key > min_keys)
        {
            cessor = override_successor(current_node, current);
            delete_value(cessor, current_node->child[current + 1]);
        }
        else
        {
            deleted_for_merge = merge_child(current_node, current);
            delete_value(deleted_for_merge, current_node->child[current]);
        }
    }
}

int delete_value(int val, struct BTreeNode *node)
{
    int pos;
    int flag = false;
    for (pos = 0; pos < node->count_key; pos++)
    {
        if (val == node->key[pos])
        {
            flag = true;
            break;
        }
        else if (val < node->key[pos])
        {
            break;
        }
    }
    if (flag)
    {
        if (node->leaf)
        {
            for (int i = pos; i < node->count_key; i++)
            {
                node->key[i] = node->key[i + 1];
            }
            node->count_key--;
        }
        else
        {
            delete_inner(node, pos);
        }
        return flag;
    }
    else
    {
        if (node->leaf)
        {
            return flag;
        }
        else
        {
            flag = delete_value(val, node->child[pos]);
        }
    }
    if (node->child[pos]->count_key < min_keys)
    {
        balance(node, pos);
    }

    return flag;
}

void delete (struct BTreeNode *node, int val)
{
    if (!node)
    {
        return;
    }
    int flag = delete_value(val, node);
    if (!flag)
    {
        return;
    }
    if (node->count_key == 0)
    {
        node = node->child[0];
    }
    root = node;
}

void shuffle(int *arr, int num)
{
    srand((unsigned)time(NULL));
    int temp;
    int ran;
    for (int i = 0; i < num - 1; i++)
    {
        ran = rand() % (num - i) + i;
        temp = arr[i];
        arr[i] = arr[ran];
        arr[ran] = temp;
    }
}

int main(void)
{

    LARGE_INTEGER tickPerSecond;

    LARGE_INTEGER startTick_insert, endTick_insert, startTick_delete, endTick_delete;

    double insert_time, delete_time;

    QueryPerformanceFrequency(&tickPerSecond);

    static int value[10000];
    int random_value[1000];
    int delete_index = rand() % 500;
    int count = 0;
    for (int i = 0; i < 10000; i++)
    {
        value[i] = i;
    }
    shuffle(value, 10000);
    for (int i = 0; i < 1000; i++)
    {
        random_value[i] = value[i];
    }
    QueryPerformanceCounter(&startTick_insert);
    for (int i = 0; i < 1000; i++)
    {
        insert(random_value[i]);
    }
    QueryPerformanceCounter(&endTick_insert);
    insert_time = (double)(endTick_insert.QuadPart - startTick_insert.QuadPart) / tickPerSecond.QuadPart;
    printf("Insert Time : %lf\n", insert_time);

    QueryPerformanceCounter(&startTick_delete);
    for (int i = delete_index; i < delete_index + 500; i++)
    {
        delete (root, random_value[i]);
    }
    QueryPerformanceCounter(&endTick_delete);
    delete_time = (double)(endTick_delete.QuadPart - startTick_delete.QuadPart) / tickPerSecond.QuadPart;
    printf("Delete Time : %lf\n", delete_time);

    return 0;
}