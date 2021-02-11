#pragma once

typedef struct list_node
{
    void* data;
    struct list_node* next;
} list_node;

typedef struct list
{
    list_node* first;
} list;

/// Inserts a new node into the list as list_node* at's next
void list_insert(void* data, list_node* at)
{
    if (!at)
        return;
    
    
}