#include "utils/comap/comap.h"
#include "utils/memanager/context.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>

#define MAX_CAPACITY 1 << 30

typedef enum
{
    MOVED = -1,
    // ERROR_INVALID, // -2
    // ERROR_TIMEOUT, // -3
    // ERROR_MEMORY   // -4
} NodeState;

typedef struct Node
{
    void *key;
    void *val;
    struct Node *next;
    int hash;
} Node;

typedef struct HeadNode
{
    Node node;
    pthread_mutex_t lock;
} HeadNode;

typedef struct CoHmap
{
    int capacity;
    int keySize;
    _Atomic(struct Node *) *table;
    _Atomic(struct Node *) *next_table;
    MemoryContext *context;
} CoHmap;

void PrintCoHmap(CoHmap *map)
{
    if (map == NULL)
    {
        printf("Map is NULL\n");
        return;
    }

    printf("=== CoHmap (capacity: %d) ===\n", map->capacity);

    int totalNodes = 0;

    for (int i = 0; i < map->capacity; i++)
    {
        // Load the atomic pointer
        Node *head = atomic_load(&map->table[i]);

        if (head != NULL)
        {
            printf("Bucket[%d]: ", i);

            // Check if it's a HeadNode (has the lock)
            HeadNode *headNode = (HeadNode *)head;
            Node *curr = &headNode->node;

            int chainLength = 0;
            while (curr != NULL)
            {
                chainLength++;
                totalNodes++;

                // Print based on your key type
                // For integer keys:
                if (map->keySize == sizeof(int))
                {
                    printf("[hash:%d, key:%d, val:%d]",
                           curr->hash,
                           *(int *)curr->key,
                           *(int *)curr->val);
                }
                // For string keys:
                else if (curr->key != NULL)
                {
                    printf("[hash:%d, key:%s, val:%p]",
                           curr->hash,
                           (char *)curr->key,
                           curr->val);
                }
                else
                {
                    printf("[hash:%d, key:%p, val:%p]",
                           curr->hash,
                           curr->key,
                           curr->val);
                }

                curr = curr->next;

                if (curr != NULL)
                    printf(" -> ");
            }

            printf(" (chain length: %d)\n", chainLength);
        }
    }

    printf("Total nodes: %d\n", totalNodes);
    printf("========================\n");
}

void panic2(const char *msg)
{
    fprintf(stderr, "PANIC: %s\n", msg);
    exit(1);
}

unsigned int next_pow2(unsigned int n)
{
    if (n == 0)
        return 1;

    if ((n & (n - 1)) == 0)
        return n;

    int leading_zeros = __builtin_clz(n);
    int pos = 32 - leading_zeros;
    return 1u << pos;
}

CoHmap *CreateCoHmap(int initialCapacity, float loadFactor, int keySize)
{
    if (initialCapacity <= 0 || loadFactor <= 0)
    {
        panic2("size or load cant be less than or equal to 0");
    }

    int size = (int)(1.0 + initialCapacity / loadFactor);
    if (size > MAX_CAPACITY)
    {
        size = MAX_CAPACITY;
    }
    int capacity = next_pow2(size);

    MemoryContext *context = CreateSetAllocContextInternal("cohmap");
    Block map_block = context->methods->alloc(context, sizeof(CoHmap));
    CoHmap *map = map_block.data;
    Block arr_block = context->methods->alloc(context, capacity * sizeof(void *));

    map->table = arr_block.data;
    map->capacity = capacity;
    map->context = context;
    map->keySize = keySize;
    map->next_table = NULL;

    for (int i = 0; i < capacity; i++)
    {
        atomic_init(&map->table[i], NULL);
    }

    return map;
}

struct Node *get_at(CoHmap *map, int i)
{
    return atomic_load_explicit(&map->table[i], memory_order_acquire);
}

void set_at(CoHmap *map, int i, struct Node *node)
{
    atomic_store_explicit(&map->table[i], node, memory_order_release);
}

bool cas_at(CoHmap *map, int i, struct Node *expected, struct Node *desired)
{
    return atomic_compare_exchange_strong_explicit(
        &map->table[i],
        &expected,
        desired,
        memory_order_release,
        memory_order_acquire);
}

int calculate_hash(void *key, int size) // TODO
{
    unsigned char *bytes = (unsigned char *)key;
    unsigned int h = 0;

    for (int i = 0; i < size; i++)
    {
        h = 31 * h + bytes[i];
    }

    return (int)h;
}

bool equals(void *key1, void *key2)
{
    return *(int *)key1 == *(int *)key2; // TODO
}

void *Get(CoHmap *map, void *key)
{
    int hash = calculate_hash(key, map->keySize);
    if (map->capacity == 0)
    {
        return NULL;
    }

    Node *item = get_at(map, hash & (map->capacity - 1));
    if (item == NULL)
    {
        return NULL;
    }

    if (item->hash == hash && (item->key == key || equals(item->key, key)))
    {
        return item->val;
    }

    while (item->next != NULL)
    {
        item = item->next;
        if (item->hash == hash && (item->key == key || equals(item->key, key)))
        {
            return item->val;
        }
    }

    return NULL;
}

bool Contains(CoHmap *map, void *key)
{
    return Get(map, key) != NULL;
}

void putItem(CoHmap *map, Node *curr, int hash, void *key, void *value)
{
    if (curr->hash == hash && (curr->key == key || equals(curr->key, key)))
    {
        curr->val = value;
        return;
    }

    while (curr->next != NULL)
    {
        curr = curr->next;
        if (curr->hash == hash && (curr->key == key || equals(curr->key, key)))
        {
            curr->val = value;
            return;
        }
    }

    Block block = map->context->methods->alloc(map->context, sizeof(Node));
    Node *newNode = (Node *)block.data;
    newNode->hash = hash;
    newNode->key = key;
    newNode->val = value;
    newNode->next = NULL;
    curr->next = newNode;
}

void Put(CoHmap *map, void *key, void *value)
{
    if (key == NULL || value == NULL)
        panic2("key/value is NULL");

    int hash = calculate_hash(key, map->keySize);
    int idx = hash & (map->capacity - 1);
    while (true)
    {
        HeadNode *item = (HeadNode *)get_at(map, idx);
        if (item == NULL)
        {
            Block newBlock = map->context->methods->alloc(map->context, sizeof(HeadNode));
            HeadNode *newNode = (HeadNode *)newBlock.data;
            newNode->node.hash = hash;
            newNode->node.key = key;
            newNode->node.val = value;
            newNode->node.next = NULL;
            pthread_mutex_init(&newNode->lock, NULL);
            if (cas_at(map, idx, NULL, (Node *)newNode))
            {
                break;
            }
        }
        else if (item->node.hash == MOVED)
        {
            // TODO MOVED
        }
        else
        {
            Node *curr = &item->node;
            pthread_mutex_lock(&item->lock);

            HeadNode *current_head = (HeadNode *)get_at(map, idx);
            if (current_head != item)
            {
                pthread_mutex_unlock(&item->lock);
                continue;
            }
            putItem(map, curr, hash, key, value);

            pthread_mutex_unlock(&item->lock);
            break;
        }
    }
}
