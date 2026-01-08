#include "utils/memanager/context.h"
#include <stdio.h>
#include <stdlib.h>

_Thread_local MemoryContext *CURRENT_CONTEXT = NULL;

void SwitchTo(MemoryContext *context)
{
    CURRENT_CONTEXT = context;
}

Block Alloc(uint32 size)
{
    return CURRENT_CONTEXT->methods->alloc(CURRENT_CONTEXT, size);
}

void Free(Block block)
{
    CURRENT_CONTEXT->methods->jfree(CURRENT_CONTEXT, block);
}

void Delete()
{
    UnlinkFromParent(CURRENT_CONTEXT);
    CURRENT_CONTEXT->methods->delete(CURRENT_CONTEXT);
    SwitchTo(NULL);
}

void UnlinkFromParent(MemoryContext *context)
{
    MemoryContext *parent = context->parent;

    if (parent == NULL)
        return;

    MemoryContext **prev = &parent->children;
    while (*prev && *prev != context)
    {
        prev = &(*prev)->next;
    }

    if (*prev == context)
    {
        *prev = context->next;
    }

    context->parent = NULL;
    context->next = NULL;
}