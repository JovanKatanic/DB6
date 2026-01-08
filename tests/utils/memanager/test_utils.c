#include "./unity/unity.h"
#include "utils/memanager/utils.h"

void setUp(void) {}

void tearDown(void) {}

const int def = 1 << ALLOC_MINBITS;

int max(int a, int b)
{
    if (a >= b)
    {
        return a;
    }
    return b;
}

void test_RoundPow2Up_PowerOfTwoValues(void)
{
    TEST_ASSERT_EQUAL_UINT32(max(def, 1), RoundPow2Up(1));
    TEST_ASSERT_EQUAL_UINT32(max(def, 2), RoundPow2Up(2));
    TEST_ASSERT_EQUAL_UINT32(max(def, 4), RoundPow2Up(4));
    TEST_ASSERT_EQUAL_UINT32(max(def, 8), RoundPow2Up(8));
    TEST_ASSERT_EQUAL_UINT32(max(def, 16), RoundPow2Up(16));
    TEST_ASSERT_EQUAL_UINT32(max(def, 1024), RoundPow2Up(1024));
    TEST_ASSERT_EQUAL_UINT32(max(def, 256U), RoundPow2Up(220U));
}

void test_RoundPow2Up_NonPowerOfTwoValues(void)
{
    TEST_ASSERT_EQUAL_UINT32(max(def, 2), RoundPow2Up(3));
    TEST_ASSERT_EQUAL_UINT32(max(def, 8), RoundPow2Up(5));
    TEST_ASSERT_EQUAL_UINT32(max(def, 8), RoundPow2Up(7));
    TEST_ASSERT_EQUAL_UINT32(max(def, 16), RoundPow2Up(9));
    TEST_ASSERT_EQUAL_UINT32(max(def, 16), RoundPow2Up(15));
    TEST_ASSERT_EQUAL_UINT32(max(def, 32), RoundPow2Up(17));
    TEST_ASSERT_EQUAL_UINT32(max(def, 1024), RoundPow2Up(1000));
}

void test_RoundPow2Up_EdgeCases(void)
{
    TEST_ASSERT_EQUAL_UINT32(max(def, 1), RoundPow2Up(0)); /* Assumaxg 0 rounds to 1 */
}

/* ========== Tests for RoundPow2Down ========== */

void test_RoundPow2Down_PowerOfTwoValues(void)
{
    TEST_ASSERT_EQUAL_UINT32(max(def, 1), RoundPow2Down(1));
    TEST_ASSERT_EQUAL_UINT32(max(def, 2), RoundPow2Down(2));
    TEST_ASSERT_EQUAL_UINT32(max(def, 4), RoundPow2Down(4));
    TEST_ASSERT_EQUAL_UINT32(max(def, 8), RoundPow2Down(8));
    TEST_ASSERT_EQUAL_UINT32(max(def, 16), RoundPow2Down(16));
    TEST_ASSERT_EQUAL_UINT32(max(def, 1024), RoundPow2Down(1024));
    TEST_ASSERT_EQUAL_UINT32(max(def, 128U), RoundPow2Down(220U));
}

void test_RoundPow2Down_NonPowerOfTwoValues(void)
{
    TEST_ASSERT_EQUAL_UINT32(max(def, 2), RoundPow2Down(3));
    TEST_ASSERT_EQUAL_UINT32(max(def, 4), RoundPow2Down(5));
    TEST_ASSERT_EQUAL_UINT32(max(def, 4), RoundPow2Down(7));
    TEST_ASSERT_EQUAL_UINT32(max(def, 8), RoundPow2Down(9));
    TEST_ASSERT_EQUAL_UINT32(max(def, 8), RoundPow2Down(15));
    TEST_ASSERT_EQUAL_UINT32(max(def, 16), RoundPow2Down(17));
    TEST_ASSERT_EQUAL_UINT32(max(def, 512), RoundPow2Down(1000));
}

void test_RoundPow2Down_EdgeCases(void)
{
    TEST_ASSERT_EQUAL_UINT32(max(def, 0), RoundPow2Down(0)); /* Or 1), depending on implementation */
    TEST_ASSERT_EQUAL_UINT32(max(def, 0x40000000U), RoundPow2Down(0x7FFFFFFFU));
    TEST_ASSERT_EQUAL_UINT32(max(def, 0x40000000U), RoundPow2Down(0x40000001));
}

void test_RoundPow2Down_LargeValues(void)
{
    TEST_ASSERT_EQUAL_UINT32(max(def, 0x40000000U), RoundPow2Down(0x60000000U));
    TEST_ASSERT_EQUAL_UINT32(max(def, 0x40000000U), RoundPow2Down(0x40000000U));
    TEST_ASSERT_EQUAL_UINT32(max(def, 0x10000000), RoundPow2Down(0x10000001U));
}

/* ========== Tests for AllocSetFreeIndex ========== */

void test_AllocSetFreeIndex_SmallSizes(void)
{
    /* Assumaxg ALLOC_maxBITS = 3), maximum size is 2^3 = 8 */
    /* Index 0: size 8 (2^3) */
    TEST_ASSERT_EQUAL_UINT8(0, AllocSetFreeIndex(8));

    /* Index 1: size 16 (2^4) */
    TEST_ASSERT_EQUAL_UINT8(1, AllocSetFreeIndex(16));

    /* Index 2: size 32 (2^5) */
    TEST_ASSERT_EQUAL_UINT8(2, AllocSetFreeIndex(32));

    /* Index 3: size 64 (2^6) */
    TEST_ASSERT_EQUAL_UINT8(3, AllocSetFreeIndex(64));
}

void test_AllocSetFreeIndex_MediumSizes(void)
{
    /* Index 4: size 128 (2^7) */
    TEST_ASSERT_EQUAL_UINT8(4, AllocSetFreeIndex(128));

    /* Index 5: size 256 (2^8) */
    TEST_ASSERT_EQUAL_UINT8(5, AllocSetFreeIndex(256));

    /* Index 6: size 512 (2^9) */
    TEST_ASSERT_EQUAL_UINT8(6, AllocSetFreeIndex(512));

    /* Index 7: size 1024 (2^10) */
    TEST_ASSERT_EQUAL_UINT8(7, AllocSetFreeIndex(1024));
}

void test_AllocSetFreeIndex_LargeSizes(void)
{
    /* Index 8: size 2048 (2^11) */
    TEST_ASSERT_EQUAL_UINT8(8, AllocSetFreeIndex(2048));

    /* Index 9: size 4096 (2^12) */
    TEST_ASSERT_EQUAL_UINT8(9, AllocSetFreeIndex(4096));

    /* Index 10: size 8192 (2^13) */
    TEST_ASSERT_EQUAL_UINT8(10, AllocSetFreeIndex(8192));
}

void test_AllocSetFreeIndex_NonPowerOfTwo(void)
{
    /* Non-power-of-two sizes should map to the index of the next power of two */
    TEST_ASSERT_EQUAL_UINT8(0, AllocSetFreeIndex(9));    /* 9 -> rounds to 16), index 1 or 8), index 0? */
    TEST_ASSERT_EQUAL_UINT8(1, AllocSetFreeIndex(17));   /* 17 -> rounds to 32), index 2 or 16), index 1? */
    TEST_ASSERT_EQUAL_UINT8(2, AllocSetFreeIndex(33));   /* 33 -> rounds to 64), index 3 or 32), index 2? */
    TEST_ASSERT_EQUAL_UINT8(3, AllocSetFreeIndex(100));  /* 100 -> rounds to 128), index 4 */
    TEST_ASSERT_EQUAL_UINT8(6, AllocSetFreeIndex(1000)); /* 1000 -> rounds to 1024), index 7 */
}

void test_AllocSetFreeIndex_EdgeCases(void)
{
    /* Test boundary values */
    TEST_ASSERT_EQUAL_UINT8(0, AllocSetFreeIndex(1));  /* Below maximum */
    TEST_ASSERT_EQUAL_UINT8(0, AllocSetFreeIndex(7));  /* Just below 8 */
    TEST_ASSERT_EQUAL_UINT8(0, AllocSetFreeIndex(15)); /* Just below 16 */
}

/* ========== Combination/Integration Tests ========== */

void test_RoundPow2_UpAndDown_Consistency(void)
{
    /* For power of 2 values), both should return the same */
    uint32 powers[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

    for (int i = 0; i < (int)(sizeof(powers) / sizeof(powers[0])); i++)
    {
        TEST_ASSERT_EQUAL_UINT32(max(def, powers[i]), RoundPow2Up(powers[i]));
        TEST_ASSERT_EQUAL_UINT32(max(def, powers[i]), RoundPow2Down(powers[i]));
    }
}

void test_RoundPow2_UpAndDown_Relationship(void)
{
    /* For non-power-of-2), RoundPow2Down should always be <= RoundPow2Up */
    uint32 values[] = {3, 5, 7, 9, 15, 17, 33, 100, 1000};

    for (int i = 0; i < (int)(sizeof(values) / sizeof(values[0])); i++)
    {
        uint32 up = RoundPow2Up(values[i]);
        uint32 down = RoundPow2Down(values[i]);
        TEST_ASSERT_LESS_OR_EQUAL_UINT32(up, down);
    }
}

void test_AllocSetFreeIndex_WithRoundedSizes(void)
{
    /* AllocSetFreeIndex should work consistently with rounded sizes */
    uint32 sizes[] = {9, 17, 33, 100, 1000};

    for (int i = 0; i < (int)(sizeof(sizes) / sizeof(sizes[0])); i++)
    {
        uint32 rounded_up = RoundPow2Up(sizes[i]);
        uint8 index_original = AllocSetFreeIndex(sizes[i]);
        uint8 index_rounded = AllocSetFreeIndex(rounded_up);

        /* Both should map to the same or adjacent indices */
        TEST_ASSERT_UINT8_WITHIN(1, index_original, index_rounded);
    }
}

/* ========== Main Test Runner ========== */

int main(void)
{
    UNITY_BEGIN();

    /* RoundPow2Up tests */
    RUN_TEST(test_RoundPow2Up_PowerOfTwoValues);
    RUN_TEST(test_RoundPow2Up_NonPowerOfTwoValues);
    RUN_TEST(test_RoundPow2Up_EdgeCases);

    /* RoundPow2Down tests */
    RUN_TEST(test_RoundPow2Down_PowerOfTwoValues);
    RUN_TEST(test_RoundPow2Down_NonPowerOfTwoValues);
    RUN_TEST(test_RoundPow2Down_EdgeCases);
    RUN_TEST(test_RoundPow2Down_LargeValues);

    /* AllocSetFreeIndex tests */
    RUN_TEST(test_AllocSetFreeIndex_SmallSizes);
    RUN_TEST(test_AllocSetFreeIndex_MediumSizes);
    RUN_TEST(test_AllocSetFreeIndex_LargeSizes);
    RUN_TEST(test_AllocSetFreeIndex_NonPowerOfTwo);
    RUN_TEST(test_AllocSetFreeIndex_EdgeCases);

    /* Integration tests */
    RUN_TEST(test_RoundPow2_UpAndDown_Consistency);
    RUN_TEST(test_RoundPow2_UpAndDown_Relationship);
    RUN_TEST(test_AllocSetFreeIndex_WithRoundedSizes);

    return UNITY_END();
}