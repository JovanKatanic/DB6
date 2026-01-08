#include "./unity/unity.h"
#include "utils/memanager/context.h"
#include <stdbool.h>

void setUp(void) {}
void tearDown(void) {}

void test_create_context(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    TEST_ASSERT_NOT_NULL(ctx);
    TEST_ASSERT_EQUAL_STRING("test", ctx->name);
}

void test_basic_allocation(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");
    Block b = Alloc(100);

    TEST_ASSERT_NOT_NULL(b.data);
    TEST_ASSERT_EQUAL(128, b.capacity); // 100 rounds up to 128

    Delete(ctx);
}

void test_allocation_rounds_to_power_of_2(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    Block b1 = Alloc(7);
    TEST_ASSERT_EQUAL(8, b1.capacity); // Rounds to 2^3

    Block b2 = Alloc(100);
    TEST_ASSERT_EQUAL(128, b2.capacity); // Rounds to 2^7

    Block b3 = Alloc(200);
    TEST_ASSERT_EQUAL(256, b3.capacity); // Rounds to 2^8

    Block b4 = Alloc(1000);
    TEST_ASSERT_EQUAL(1024, b4.capacity); // Rounds to 2^10

    Delete(ctx);
}

void test_multiple_allocations_dont_overlap(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    Block b1 = Alloc(100);
    Block b2 = Alloc(200);
    Block b3 = Alloc(300);

    TEST_ASSERT_NOT_NULL(b1.data);
    TEST_ASSERT_NOT_NULL(b2.data);
    TEST_ASSERT_NOT_NULL(b3.data);

    bool b1_b2_overlap = b1.data + 100 <= b2.data;
    bool b2_b3_overlap = b2.data + 200 <= b3.data;
    bool b1_b3_overlap = b1.data + 300 <= b3.data;

    TEST_ASSERT_TRUE(b1_b2_overlap);
    TEST_ASSERT_TRUE(b2_b3_overlap);
    TEST_ASSERT_TRUE(b1_b3_overlap);

    Delete(ctx);
}

void test_memory_alignment(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    Block b1 = Alloc(17); // Odd size
    Block b2 = Alloc(33);
    Block b3 = Alloc(99);

    // All should be 8-byte aligned
    TEST_ASSERT_EQUAL(0, (uintptr_t)b1.data % 8);
    TEST_ASSERT_EQUAL(0, (uintptr_t)b2.data % 8);
    TEST_ASSERT_EQUAL(0, (uintptr_t)b3.data % 8);

    Delete(ctx);
}

void test_large_allocation_triggers_new_block(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    // Fill up first block (INITIAL_CAPACITY = 4096)
    Block b1 = Alloc(3000);
    TEST_ASSERT_NOT_NULL(b1.data);

    // This should trigger new block allocation
    Block b2 = Alloc(3000);
    TEST_ASSERT_NOT_NULL(b2.data);

    // Both should be valid
    TEST_ASSERT_NOT_EQUAL(b1.data, b2.data);

    Delete(ctx);
}

void test_free_and_reuse(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    // Allocate a block
    Block b1 = Alloc(128);
    void *original_addr = b1.data;
    TEST_ASSERT_NOT_NULL(b1.data);

    // Free it
    Free(b1);

    // Allocate same size - should reuse
    Block b2 = Alloc(100);
    TEST_ASSERT_EQUAL(original_addr, b2.data);
    TEST_ASSERT_EQUAL(128, b2.capacity);

    Delete(ctx);
}

void test_free_different_sizes(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    Block b64 = Alloc(60);
    Block b128 = Alloc(120);
    Block b256 = Alloc(256);

    void *addr64 = b64.data;
    void *addr128 = b128.data;
    void *addr256 = b256.data;

    // Free all
    Free(b64);
    Free(b128);
    Free(b256);

    // Reallocate - should reuse
    Block new64 = Alloc(60);
    Block new128 = Alloc(120);
    Block new256 = Alloc(256);

    TEST_ASSERT_EQUAL(addr64, new64.data);
    TEST_ASSERT_EQUAL(addr128, new128.data);
    TEST_ASSERT_EQUAL(addr256, new256.data);

    Delete(ctx);
}

void test_freelist_reuse_lifo(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    // Allocate multiple blocks of same size
    Block b1 = Alloc(128);
    Block b2 = Alloc(128);
    Block b3 = Alloc(128);

    void *addr1 = b1.data;
    void *addr2 = b2.data;
    void *addr3 = b3.data;

    // Free in order: 1, 2, 3
    Free(b1);
    Free(b2);
    Free(b3);

    // Reallocate - should get in LIFO order: 3, 2, 1
    Block new1 = Alloc(128);
    Block new2 = Alloc(128);
    Block new3 = Alloc(128);

    TEST_ASSERT_EQUAL(addr3, new1.data); // Last freed, first reused
    TEST_ASSERT_EQUAL(addr2, new2.data);
    TEST_ASSERT_EQUAL(addr1, new3.data);

    Delete(ctx);
}

void test_mixed_alloc_and_free(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    Block blocks[10];

    // Allocate 10 blocks
    for (int i = 0; i < 10; i++)
    {
        blocks[i] = Alloc(64);
        TEST_ASSERT_NOT_NULL(blocks[i].data);
    }

    // Free half of them
    for (int i = 0; i < 5; i++)
    {
        Free(blocks[i]);
    }

    // Allocate more - should reuse freed blocks
    for (int i = 0; i < 5; i++)
    {
        Block b = Alloc(64);
        TEST_ASSERT_NOT_NULL(b.data);
    }

    Delete(ctx);
}

void test_zero_size_allocation(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    Block b = Alloc(0);

    // Should allocate minimum size (8 bytes)
    TEST_ASSERT_NOT_NULL(b.data);
    TEST_ASSERT_EQUAL(8, b.capacity);

    Delete(ctx);
}

void test_minimum_size_allocation(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    Block b1 = Alloc(1);
    Block b2 = Alloc(7);
    Block b3 = Alloc(8);

    // All should round to minimum size (8)
    TEST_ASSERT_EQUAL(8, b1.capacity);
    TEST_ASSERT_EQUAL(8, b2.capacity);
    TEST_ASSERT_EQUAL(8, b3.capacity);

    Delete(ctx);
}

void test_stress_many_allocations(void)
{
    MemoryContext *ctx = CreateSetAllocContext("test");

    Block blocks[100];

    // Allocate 100 blocks of varying sizes
    for (int i = 0; i < 100; i++)
    {
        blocks[i] = Alloc((i + 1) * 10);
        TEST_ASSERT_NOT_NULL(blocks[i].data);
    }

    // Free every other one
    for (int i = 0; i < 100; i += 2)
    {
        Free(blocks[i]);
    }

    // Allocate more
    for (int i = 0; i < 50; i++)
    {
        Block b = Alloc(100);
        TEST_ASSERT_NOT_NULL(b.data);
    }

    Delete(ctx);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_basic_allocation);
    RUN_TEST(test_allocation_rounds_to_power_of_2);
    RUN_TEST(test_multiple_allocations_dont_overlap);
    RUN_TEST(test_memory_alignment);
    RUN_TEST(test_large_allocation_triggers_new_block);
    RUN_TEST(test_free_and_reuse);
    RUN_TEST(test_free_different_sizes);
    RUN_TEST(test_freelist_reuse_lifo);
    RUN_TEST(test_mixed_alloc_and_free);
    RUN_TEST(test_zero_size_allocation);
    RUN_TEST(test_minimum_size_allocation);
    RUN_TEST(test_stress_many_allocations);

    return UNITY_END();
}