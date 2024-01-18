// SPDX-FileCopyrightText: 2024 Davide Paro
//
// SPDX-License-Identifier: MIT

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

/* A test case that does nothing and succeeds. */
static void test_1(void **state) {
    (void)state; /* unused */
    *((int32_t *)0) = 0;
}

static void test_2(void **state) {
    (void)state; /* unused */
    assert_int_equal(2, 1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_1),
        cmocka_unit_test(test_2),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
