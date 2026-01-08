#include "utils/comap/comap.h"

typedef struct Node
{
    void *key;
    void *val;
    void *next;
    int hash;
} Node;
