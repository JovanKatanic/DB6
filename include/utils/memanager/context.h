#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include "utils/memanager/alloc_defs.h"

typedef struct MemoryContext MemoryContext;

// .
// Block should never be created or edited outside of this block
// .
typedef struct Block
{
    void *data;
    uint32 capacity;
} Block;

typedef struct MemoryContextMethods
{
    Block (*alloc)(MemoryContext *, uint32);
    void (*jfree)(MemoryContext *, Block);
    void (*delete)(MemoryContext *);
} MemoryContextMethods;

typedef struct MemoryContext
{
    char *name;
    MemoryContext *parent;
    MemoryContext *children;
    MemoryContext *next;
    MemoryContextMethods *methods;
} MemoryContext;

extern _Thread_local MemoryContext *CURRENT_CONTEXT;

/* Public API */
MemoryContext *CreateSetAllocContext(char *name);
MemoryContext *CreateSetAllocContextInternal(char *name);
MemoryContext *CreateSlabAllocContext(char *name, uint32 slabSize);
Block Alloc(uint32 size);
void Free(Block block);
void Delete();
void SwitchTo(MemoryContext *context);

/* Utility functions */
void UnlinkFromParent(MemoryContext *context);

#endif /* CONTEXT_H */