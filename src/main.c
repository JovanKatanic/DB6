#include "utils/memanager/context.h"
#include <stdio.h>

MemoryContext *createSetContext(char *name)
{
    printf("new context \n");
    MemoryContext *context = CreateSetAllocContext(name);
    Block ptr1 = Alloc(100);
    Block ptr2 = Alloc(2000);

    printf("ptr2: %ld\n", (uint8_t *)ptr2.data - (uint8_t *)ptr1.data);

    Block ptr3 = Alloc(3000);
    Block ptr4 = Alloc(1000);
    printf("ptr4: %ld\n", (uint8_t *)ptr4.data - (uint8_t *)ptr3.data);

    Free(ptr3);
    Block ptr5 = Alloc(3000);
    printf("ptr6: %ld\n", (uint8_t *)ptr5.data - (uint8_t *)ptr3.data);

    return context;
}

MemoryContext *createSlabContext(char *name)
{
    printf("new context \n");
    MemoryContext *context = CreateSlabAllocContext(name, 2000);
    Block ptr1 = Alloc(0);
    Block ptr2 = Alloc(0);

    printf("ptr2: %ld\n", (uint8_t *)ptr2.data - (uint8_t *)ptr1.data);

    Block ptr3 = Alloc(0);
    Block ptr4 = Alloc(0);
    printf("ptr4: %ld\n", (uint8_t *)ptr4.data - (uint8_t *)ptr3.data);

    Free(ptr3);
    Block ptr5 = Alloc(0);
    printf("ptr6: %ld\n", (uint8_t *)ptr5.data - (uint8_t *)ptr3.data);

    return context;
}

int main(void)
{
    MemoryContext *context = createSlabContext("context");
    MemoryContext *context_child_1 = createSlabContext("child1");
    MemoryContext *context_child_2 = createSlabContext("child2");
    printf("%p %p %p\n", (void *)context, (void *)context_child_1, (void *)context_child_2);

    SwitchTo(context_child_1);
    Delete();
    SwitchTo(context);
    Delete();

    return 0;
}