// tests/test_slab.c
#include "./unity/unity.h"
#include "utils/memanager/context.h"
#include <stdint.h>

void setUp(void)
{
    // Runs before each test
}

void tearDown(void)
{
    // Runs after each test
}

void test_create_slab_context(void)
{
    MemoryContext *ctx = CreateSlabAllocContext("test_slab", 64);

    TEST_ASSERT_NOT_NULL(ctx);
    TEST_ASSERT_EQUAL_STRING("test_slab", ctx->name);

    Delete();
}

void test_slab_basic_allocation(void)
{
    CreateSlabAllocContext("test", 64);
    Block b = Alloc(100); // Size parameter ignored

    TEST_ASSERT_NOT_NULL(b.data);
    TEST_ASSERT_EQUAL(64, b.capacity); // Always returns slab size

    Delete();
}

void test_slab_ignores_requested_size(void)
{
    CreateSlabAllocContext("test", 128);

    // Request different sizes - all should return 128
    Block b1 = Alloc(10);
    Block b2 = Alloc(500);
    Block b3 = Alloc(1000);

    TEST_ASSERT_EQUAL(128, b1.capacity);
    TEST_ASSERT_EQUAL(128, b2.capacity);
    TEST_ASSERT_EQUAL(128, b3.capacity);

    Delete();
}

void test_slab_multiple_allocations(void)
{
    CreateSlabAllocContext("test", 64);

    Block blocks[10];
    for (int i = 0; i < 10; i++)
    {
        blocks[i] = Alloc(0);
        TEST_ASSERT_NOT_NULL(blocks[i].data);
        TEST_ASSERT_EQUAL(64, blocks[i].capacity);
    }

    // All should be different addresses
    for (int i = 0; i < 9; i++)
    {
        TEST_ASSERT_NOT_EQUAL(blocks[i].data, blocks[i + 1].data);
    }

    Delete();
}

void test_slab_alignment(void)
{
    CreateSlabAllocContext("test", 64);

    Block b1 = Alloc(0);
    Block b2 = Alloc(0);
    Block b3 = Alloc(0);

    // All should be 8-byte aligned
    TEST_ASSERT_EQUAL(0, (uintptr_t)b1.data % 8);
    TEST_ASSERT_EQUAL(0, (uintptr_t)b2.data % 8);
    TEST_ASSERT_EQUAL(0, (uintptr_t)b3.data % 8);

    Delete();
}

void test_slab_size_alignment(void)
{
    // Request unaligned size
    CreateSlabAllocContext("test", 33);

    Block b = Alloc(0);

    // Should be aligned to 40 (next multiple of 8)
    TEST_ASSERT_EQUAL(40, b.capacity);

    Delete();
}

void test_slab_minimum_size(void)
{
    // Request very small size
    CreateSlabAllocContext("test", 1);

    Block b = Alloc(0);

    // Should be at least sizeof(MemSlab) = 8 bytes
    TEST_ASSERT_TRUE(b.capacity >= 8);

    Delete();
}

void test_slab_free_and_reuse(void)
{
    CreateSlabAllocContext("test", 64);

    // Allocate a slab
    Block b1 = Alloc(0);
    void *original_addr = b1.data;
    TEST_ASSERT_NOT_NULL(b1.data);

    // Free it
    Free(b1);

    // Allocate again - should reuse
    Block b2 = Alloc(0);
    TEST_ASSERT_EQUAL(original_addr, b2.data);
    TEST_ASSERT_EQUAL(64, b2.capacity);

    Delete();
}

void test_slab_freelist_lifo(void)
{
    CreateSlabAllocContext("test", 64);

    // Allocate multiple slabs
    Block b1 = Alloc(0);
    Block b2 = Alloc(0);
    Block b3 = Alloc(0);

    void *addr1 = b1.data;
    void *addr2 = b2.data;
    void *addr3 = b3.data;

    // Free in order: 1, 2, 3
    Free(b1);
    Free(b2);
    Free(b3);

    // Reallocate - should get in LIFO order: 3, 2, 1
    Block new1 = Alloc(0);
    Block new2 = Alloc(0);
    Block new3 = Alloc(0);

    TEST_ASSERT_EQUAL(addr3, new1.data); // Last freed, first reused
    TEST_ASSERT_EQUAL(addr2, new2.data);
    TEST_ASSERT_EQUAL(addr1, new3.data);

    Delete();
}

void test_slab_multiple_free_and_alloc(void)
{
    CreateSlabAllocContext("test", 64);

    Block blocks[5];

    // Allocate 5 slabs
    for (int i = 0; i < 5; i++)
    {
        blocks[i] = Alloc(0);
        TEST_ASSERT_NOT_NULL(blocks[i].data);
    }

    // Free 3 of them
    Free(blocks[0]);
    Free(blocks[2]);
    Free(blocks[4]);

    // Allocate 3 more - should reuse freed ones
    Block new1 = Alloc(0);
    Block new2 = Alloc(0);
    Block new3 = Alloc(0);

    TEST_ASSERT_EQUAL(blocks[4].data, new1.data);
    TEST_ASSERT_EQUAL(blocks[2].data, new2.data);
    TEST_ASSERT_EQUAL(blocks[0].data, new3.data);

    Delete();
}

void test_slab_triggers_new_block(void)
{
    // Small slab size to fill block quickly
    CreateSlabAllocContext("test", 64);

    // Allocate many slabs to fill first block (4096 bytes)
    // 4096 / 64 = 64 slabs
    Block blocks[100];
    for (int i = 0; i < 100; i++)
    {
        blocks[i] = Alloc(0);
        TEST_ASSERT_NOT_NULL(blocks[i].data);
        TEST_ASSERT_EQUAL(64, blocks[i].capacity);
    }

    // Should have triggered new block allocation
    // All allocations should still work
    Delete();
}

void test_slab_different_sizes(void)
{
    MemoryContext *ctx1 = CreateSlabAllocContext("slab32", 32);
    CreateSlabAllocContext("slab128", 128);
    CreateSlabAllocContext("slab256", 256);

    Block b1 = Alloc(0); // Size ignored
    Block b2 = Alloc(0);
    Block b3 = Alloc(0);

    TEST_ASSERT_EQUAL(256, b1.capacity);
    TEST_ASSERT_EQUAL(256, b2.capacity);
    TEST_ASSERT_EQUAL(256, b3.capacity);

    SwitchTo(ctx1);
    Delete();
}

void test_slab_large_size(void)
{
    // Test with large slab size
    CreateSlabAllocContext("test", 2048);

    Block b1 = Alloc(0);
    Block b2 = Alloc(0);

    TEST_ASSERT_NOT_NULL(b1.data);
    TEST_ASSERT_NOT_NULL(b2.data);
    TEST_ASSERT_EQUAL(2048, b1.capacity);
    TEST_ASSERT_EQUAL(2048, b2.capacity);
    TEST_ASSERT_NOT_EQUAL(b1.data, b2.data);

    Delete();
}

void test_slab_interleaved_alloc_free(void)
{
    CreateSlabAllocContext("test", 64);

    Block b1 = Alloc(0);
    Block b2 = Alloc(0);

    Free(b1);

    Block b3 = Alloc(0); // Should reuse b1
    TEST_ASSERT_EQUAL(b1.data, b3.data);

    Free(b2);
    Free(b3);

    Block b4 = Alloc(0); // Should reuse b3 (last freed)
    Block b5 = Alloc(0); // Should reuse b2

    TEST_ASSERT_EQUAL(b3.data, b4.data);
    TEST_ASSERT_EQUAL(b2.data, b5.data);

    Delete();
}

void test_slab_stress_many_operations(void)
{
    CreateSlabAllocContext("test", 64);

    Block blocks[200];

    // Allocate 200 slabs
    for (int i = 0; i < 200; i++)
    {
        blocks[i] = Alloc(0);
        TEST_ASSERT_NOT_NULL(blocks[i].data);
    }

    // Free every other one
    for (int i = 0; i < 200; i += 2)
    {
        Free(blocks[i]);
    }

    // Allocate 100 more
    for (int i = 0; i < 100; i++)
    {
        Block b = Alloc(0);
        TEST_ASSERT_NOT_NULL(b.data);
    }

    Delete();
}

void test_slab_all_freed_then_realloc(void)
{
    CreateSlabAllocContext("test", 64);

    Block blocks[10];
    void *addrs[10];

    // Allocate 10 slabs
    for (int i = 0; i < 10; i++)
    {
        blocks[i] = Alloc(0);
        addrs[i] = blocks[i].data;
    }

    // Free all of them
    for (int i = 0; i < 10; i++)
    {
        Free(blocks[i]);
    }

    // Reallocate 10 slabs
    for (int i = 0; i < 10; i++)
    {
        blocks[i] = Alloc(0);
        TEST_ASSERT_NOT_NULL(blocks[i].data);
    }

    // All should be reused addresses (in LIFO order)
    TEST_ASSERT_EQUAL(addrs[9], blocks[0].data);
    TEST_ASSERT_EQUAL(addrs[8], blocks[1].data);
    TEST_ASSERT_EQUAL(addrs[7], blocks[2].data);

    Delete();
}

void test_slab_consistent_size_after_free(void)
{
    CreateSlabAllocContext("test", 128);

    Block b1 = Alloc(0);
    TEST_ASSERT_EQUAL(128, b1.capacity);

    Free(b1);

    Block b2 = Alloc(0);
    TEST_ASSERT_EQUAL(128, b2.capacity);
    TEST_ASSERT_EQUAL(b1.data, b2.data);

    Delete();
}

// Main test runner
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_create_slab_context);
    RUN_TEST(test_slab_basic_allocation);
    RUN_TEST(test_slab_ignores_requested_size);
    RUN_TEST(test_slab_multiple_allocations);
    RUN_TEST(test_slab_alignment);
    RUN_TEST(test_slab_size_alignment);
    RUN_TEST(test_slab_minimum_size);
    RUN_TEST(test_slab_free_and_reuse);
    RUN_TEST(test_slab_freelist_lifo);
    RUN_TEST(test_slab_multiple_free_and_alloc);
    RUN_TEST(test_slab_triggers_new_block);
    RUN_TEST(test_slab_different_sizes);
    RUN_TEST(test_slab_large_size);
    RUN_TEST(test_slab_interleaved_alloc_free);
    RUN_TEST(test_slab_stress_many_operations);
    RUN_TEST(test_slab_all_freed_then_realloc);
    RUN_TEST(test_slab_consistent_size_after_free);

    return UNITY_END();
}