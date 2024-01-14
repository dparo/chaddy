// SPDX-FileCopyrightText: 2023 Davide Paro
//
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unity.h>

#include "sum.h"
#include <string.h>

void setUp(void)
{
  /* This is run before EACH TEST */
}

void tearDown(void)
{
}


static void test_example(void) {
    TEST_ASSERT_EQUAL(3, 1 + 2);
}

static void test_calling_fread(void) {
    FILE *f = fopen("./CMakeLists.txt", "r");
    char buffer[16 * 1024];
    size_t sizeRead = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);
    TEST_ASSERT_GREATER_OR_EQUAL(64, sizeRead);
}

static void test_calling_calloc_0_0(void) {
    char *p = calloc(0, 0);
    TEST_ASSERT_NOT_NULL(p);
    free(p);
}

static void test_calling_malloc_0(void) {
    char *p = malloc(0);
    TEST_ASSERT_NOT_NULL(p);
    free(p);
}

static void test_calling_sum(void) {
    for (int32_t i = -10; i < 10; i++) {
        for (int32_t j = -10; j < 10; j++) {
            TEST_ASSERT_EQUAL(i + j, sum(i, j));
        }
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_example);
    RUN_TEST(test_calling_fread);
    RUN_TEST(test_calling_calloc_0_0);
    RUN_TEST(test_calling_malloc_0);
    RUN_TEST(test_calling_sum);;
    return UNITY_END();
}
