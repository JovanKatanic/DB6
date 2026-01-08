#include "utils/comap/comap.h"
#include "utils/memanager/context.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_CAPACITY 1 << 31

typedef struct Node
{
    void *key;
    void *val;
    struct Node *next;
    int hash;
} Node;

typedef struct CoHmap
{
    int capacity;
    _Atomic(struct Node *) *table;
    _Atomic(struct Node *) *next_table;
} CoHmap;

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

CoHmap *Create(int initialCapacity, float loadFactor)
{
    if (initialCapacity <= 0 || loadFactor <= 0)
    {
        panic("size or load cant be less than or equal to 0");
    }

    int size = (int)(1.0 + initialCapacity / loadFactor);
    if (size > MAX_CAPACITY)
    {
        size = MAX_CAPACITY;
    }
    int capacity = next_pow2(size);

    Block map_block = Alloc(sizeof(CoHmap));
    CoHmap *map = map_block.data;
    Block arr_block = Alloc(capacity * sizeof(Node *));
    map->capacity = capacity;
    map->table = arr_block.data;

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

int calculate_hash(char *key, int size) // TODO
{
    int h = 0;
    for (int i = 0; i < size; i++)
    {
        h = 31 * h + key[i];
    }
    return h;
}

bool equals(void *key1, void *key2)
{
    return key1 == key2; // TODO
}

void *Get(CoHmap *map, void *key, int keySize)
{
    int hash = calculate_hash(key, keySize);
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

bool Contains(CoHmap *map, void *key, int keySize)
{
    return Get(map, key, keySize) != NULL;
}

void Put(CoHmap *map, void *key, int keySize, void *value, int valueSize)
{
    if (key == NULL || value == NULL)
        panic("key/value is NULL");

    int hash = calculate_hash(key, keySize);
    int binCount = 0;

    int idx = hash & (map->capacity - 1);
    Node *item = get_at(map, idx);
    while (true)
    {
        if (item == NULL)
        {
            Block newBlock = Alloc(sizeof(Node));
            if (cas_at(map, idx, NULL, newBlock.data))
            {
                break;
            }
        }
        else if (true)
        {
            // TODO MOVED
        }
    }
}