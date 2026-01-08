#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "utils/memanager/alloc_defs.h"

/* Utility functions */
void panic(const char *msg);
uint8 AllocSetFreeIndex(uint32 size);
uint32 RoundPow2Up(uint32 size);
uint32 RoundPow2Down(uint32 size);

#endif /* UTILS_H */