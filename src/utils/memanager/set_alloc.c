#include "utils/memanager/utils.h"
#include "utils/memanager/context.h"
#include <stdio.h>
#include <stdlib.h>

#define ALLOCSET_NUM_FREELISTS 11

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

typedef struct MemorySetContext
{
    MemoryContext base;
    MemBlock *blocks;
    MemBlock *currblock;
    MemSlab *freelist[ALLOCSET_NUM_FREELISTS];
} MemorySetContext;

static Block alloc(MemoryContext *context, uint32 size);
static void jfree(MemoryContext *ctx, Block block);
static void delete(MemoryContext *context);

static MemoryContextMethods AllocSetMethods = {
    .alloc = alloc,
    .jfree = jfree,
    .delete = delete,
};

MemoryContext *CreateSetAllocContext(char *name)
{
    uint32 block_header = MAXALIGN(sizeof(MemBlock));
    uint32 context_header = MAXALIGN(sizeof(MemorySetContext));
    uint32 tot_capacity = block_header + context_header + INITIAL_CAPACITY;

    uint8 *buffer = malloc(tot_capacity);
    if (buffer == NULL)
        panic("Failed to allocate block");

    // Calculate offsets of block and context metadata

    MemBlock *block = (MemBlock *)buffer;

    uint8 *context_start = buffer + block_header;

    MemorySetContext *context = (MemorySetContext *)context_start;

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
        .methods = &AllocSetMethods,
        .children = NULL,
        .next = NULL};

    context->base = base;
    context->blocks = block;
    context->currblock = block;

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

static Block alloc(MemoryContext *ctx, uint32 size)
{
    MemorySetContext *context = (MemorySetContext *)ctx;

    void *addr;
    size = RoundPow2Up(size);
    uint8 type = AllocSetFreeIndex(size);
    MemSlab *slab = context->freelist[type];

    if (slab == NULL)
    {
        if (context->currblock->curr + size > context->currblock->end)
        {
            uint32 freeLeft = context->currblock->end - context->currblock->curr;
            if (freeLeft >= (1 << ALLOC_MINBITS))
            {
                freeLeft = RoundPow2Down(freeLeft);
                uint8 *slabLoc = context->currblock->curr;
                MemSlab *freeSlab = (MemSlab *)slabLoc;
                uint8 freeType = AllocSetFreeIndex(freeLeft);
                freeSlab->next = context->freelist[freeType];
                context->freelist[freeType] = freeSlab;
            }

            uint32 lastCap = context->currblock->capacity << 1;
            MemBlock *block = initBlock(lastCap);
            context->currblock->next = block;
            context->currblock = block;
        }

        addr = context->currblock->curr;
        context->currblock->curr += size;
    }
    else
    {
        context->freelist[type] = slab->next;
        addr = slab;
    }

    Block result = {
        .data = addr,
        .capacity = size};

    return result;
}

static void delete(MemoryContext *ctx)
{
    if (ctx == NULL)
        return;

    delete(ctx->next);
    delete(ctx->children);

    MemorySetContext *context = (MemorySetContext *)ctx;

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
    MemorySetContext *context = (MemorySetContext *)ctx;
    uint8 type = AllocSetFreeIndex(block.capacity);

    MemSlab *slab = (MemSlab *)block.data;
    slab->next = context->freelist[type];
    context->freelist[type] = slab;
}
