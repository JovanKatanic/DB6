#ifndef ALLOC_DEFS_H
#define ALLOC_DEFS_H

#include <stdint.h>

typedef uint32_t uint32;
typedef uint8_t uint8;

#define ALLOC_MINBITS 3

#define INITIAL_CAPACITY 4096 // should be power of 2

#define MAXIMUM_ALIGNOF 8
#define MAXALIGN(LEN) \
    (((uintptr_t)(LEN) + (MAXIMUM_ALIGNOF - 1)) & ~((uintptr_t)(MAXIMUM_ALIGNOF - 1)))

#endif