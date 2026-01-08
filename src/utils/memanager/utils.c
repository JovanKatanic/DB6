#include "utils/memanager/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void panic(const char *msg)
{
    fprintf(stderr, "PANIC: %s\n", msg);
    exit(1);
}

inline uint8 AllocSetFreeIndex(uint32 size)
{
    if (size <= (1 << ALLOC_MINBITS))
        return 0;

    return 31 - __builtin_clz(size) - ALLOC_MINBITS;
}

inline uint32 RoundPow2Up(uint32 size)
{
    if (size <= (1 << ALLOC_MINBITS))
        return (1 << ALLOC_MINBITS);

    return 1 << (32 - __builtin_clz(size - 1));
}

inline uint32 RoundPow2Down(uint32 size)
{
    if (size <= (1 << ALLOC_MINBITS))
        return (1 << ALLOC_MINBITS);

    return 1 << (31 - __builtin_clz(size));
}
