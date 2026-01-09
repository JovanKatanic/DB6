#include "utils/memanager/context.h"
#include "utils/memanager/utils.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct MemBlock
{
    uint32 capacity;
    uint8 *start;
    uint8 *curr;
    uint8 *end;
    struct MemBlock *next;
} MemBlock;

typedef struct MemSlab
{
    struct MemSlab *next;
} MemSlab;

typedef struct MemorySlabContext
{
    MemoryContext base;
    MemBlock *blocks;
    MemBlock *currblock;
    MemSlab *freelist;
    uint32 slabSize;
} MemorySlabContext;

static Block alloc(MemoryContext *context, uint32 size);
static void jfree(MemoryContext *ctx, Block block);
static void delete(MemoryContext *context);

static MemoryContextMethods AllocMethods = {
    .alloc = alloc,
    .jfree = jfree,
    .delete = delete,
};

MemoryContext *CreateSlabAllocContext(char *name, uint32 slabSize)
{
    uint32 block_header = MAXALIGN(sizeof(MemBlock));
    uint32 context_header = MAXALIGN(sizeof(MemorySlabContext));
    uint32 tot_capacity = block_header + context_header + INITIAL_CAPACITY;

    uint8 *buffer = malloc(tot_capacity);
    if (buffer == NULL)
        panic("Failed to allocate block");

    // Calculate offsets of block and context metadata

    MemBlock *block = (MemBlock *)buffer;

    uint8 *context_start = buffer + block_header;

    MemorySlabContext *context = (MemorySlabContext *)context_start;

    uint8 *data_start = context_start + context_header;

    // Initialize values for block and context metadata

    block->capacity = INITIAL_CAPACITY;
    block->start = buffer;
    block->curr = data_start;
    block->end = buffer + tot_capacity;
    block->next = NULL;

    MemoryContext base = {
        .name = name,
        .parent = CURRENT_CONTEXT,
        .methods = &AllocMethods,
        .children = NULL,
        .next = NULL};

    context->base = base;
    context->blocks = block;
    context->currblock = block;
    context->slabSize = MAXALIGN(slabSize);
    context->freelist = NULL;

    // Change parent state so it contains this node as child
    // for later removal when free is called

    MemoryContext *ctx = (MemoryContext *)context;

    if (CURRENT_CONTEXT != NULL)
    {
        if (CURRENT_CONTEXT->children == NULL)
        {
            CURRENT_CONTEXT->children = ctx;
        }
        else
        {
            MemoryContext *curr = CURRENT_CONTEXT->children;
            while (curr->next != NULL)
            {
                curr = curr->next;
            }
            curr->next = ctx;
        }
    }

    SwitchTo(ctx);

    return ctx;
}

static MemBlock *initBlock(uint32 capacity)
{
    uint32 header = MAXALIGN(sizeof(MemBlock));

    uint8 *buffer = malloc(header + capacity);
    if (buffer == NULL)
        panic("Failed to allocate block");

    MemBlock *block = (MemBlock *)buffer;

    uint8 *data_start = buffer + header;

    block->capacity = capacity;
    block->start = buffer;
    block->curr = data_start;
    block->end = data_start + capacity;
    block->next = NULL;

    return block;
}

static Block alloc(MemoryContext *ctx, uint32 unused_size __attribute__((unused)))
{
    MemorySlabContext *context = (MemorySlabContext *)ctx;

    void *addr;
    uint32 size = context->slabSize;
    MemSlab *slab = context->freelist;

    if (slab == NULL)
    {
        if (context->currblock->curr + size > context->currblock->end)
        {
            uint32 lastCap = context->currblock->capacity;
            MemBlock *block = initBlock(lastCap);
            context->currblock->next = block;
            context->currblock = block;
        }

        addr = context->currblock->curr;
        context->currblock->curr += size;
    }
    else
    {
        context->freelist = slab->next;
        addr = slab;
    }

    Block result = {
        .data = addr,
        .capacity = context->slabSize};

    return result;
}

static void delete(MemoryContext *ctx)
{
    if (ctx == NULL)
        return;

    delete(ctx->next);
    delete(ctx->children);

    MemorySlabContext *context = (MemorySlabContext *)ctx;

    MemBlock *curr = context->blocks;
    while (curr != NULL)
    {
        MemBlock *next = curr->next;
        free(curr->start);
        curr = next;
    }
}

static void jfree(MemoryContext *ctx, Block block)
{
    MemorySlabContext *context = (MemorySlabContext *)ctx;
    MemSlab *slab = (MemSlab *)block.data;
    slab->next = context->freelist;
    context->freelist = slab;
}
