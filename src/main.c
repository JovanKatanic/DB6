#include "utils/memanager/context.h"
#include "utils/comap/comap.h"
#include <stdio.h>
#include <pthread.h>

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
    // MemoryContext *context = createSlabContext("context");
    //  MemoryContext *context_child_1 = createSlabContext("child1");
    //  MemoryContext *context_child_2 = createSlabContext("child2");
    //  printf("%p %p %p\n", (void *)context, (void *)context_child_1, (void *)context_child_2);

    // SwitchTo(context_child_1);
    // Delete();
    // SwitchTo(context);
    // Delete();

    createSlabContext("context");

    CoHmap *map = CreateCoHmap(16, 1.0, sizeof(int));
    for (int i = 0; i < 16; i++)
    {
        Block bl1 = Alloc(sizeof(int));
        *(int *)bl1.data = i + 1;
        Block bl2 = Alloc(sizeof(int));
        *(int *)bl2.data = i + 2;
        Put(map, bl1.data, bl2.data);
    }

    PrintCoHmap(map);
    int a = 1;
    int *val = (int *)Get(map, &a);
    printf("tada %d\n", *val);

    printf("Main: Both threads created successfully\n");

    return 0;
}