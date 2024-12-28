/*
 * SPDX-FileCopyrightText: 2024 Davide Paro <dparo@outlook.it>, et al.
 * SPDX-FileContributor: Davide Paro <dparo@outlook.it>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "std.h"

static int setUp(void **_state) { /* This is run before EACH TEST */
}

static int tearDown(void **_state) {}

static void test_ptr_is_aligned(void **_state) {
    assert_true(PTR_IS_ALIGNED(NULL, 1));
    assert_true(PTR_IS_ALIGNED(NULL, 2));
    assert_true(PTR_IS_ALIGNED(NULL, 3));
    assert_true(PTR_IS_ALIGNED(NULL, 4));

    assert_true(PTR_IS_ALIGNED(1, 1));
    assert_false(PTR_IS_ALIGNED(1, 2));

    assert_true(PTR_IS_ALIGNED(4, 1));
    assert_true(PTR_IS_ALIGNED(4, 2));
    assert_false(PTR_IS_ALIGNED(4, 3));
    assert_true(PTR_IS_ALIGNED(4, 4));

    assert_true(PTR_IS_ALIGNED(0, 128));
    assert_false(PTR_IS_ALIGNED(1, 128));
    assert_false(PTR_IS_ALIGNED(2, 128));
    assert_false(PTR_IS_ALIGNED(4, 128));
    assert_false(PTR_IS_ALIGNED(64, 128));
    assert_true(PTR_IS_ALIGNED(128, 128));
    assert_true(PTR_IS_ALIGNED(256, 128));
    assert_true(PTR_IS_ALIGNED(512, 128));
}

static void test_ptr_align_up(void **_state) {
    assert_ptr_equal(0, PTR_ALIGN_UP(0, 512));
    assert_ptr_equal(512, PTR_ALIGN_UP(1, 512));
    assert_ptr_equal(512, PTR_ALIGN_UP(2, 512));
    assert_ptr_equal(512, PTR_ALIGN_UP(511, 512));
    assert_ptr_equal(512, PTR_ALIGN_UP(512, 512));
    assert_ptr_equal(1024, PTR_ALIGN_UP(513, 512));

    assert_ptr_equal(0, PTR_ALIGN_UP(0, 3));
    assert_ptr_equal(3, PTR_ALIGN_UP(1, 3));
    assert_ptr_equal(3, PTR_ALIGN_UP(3, 3));
    assert_ptr_equal(6, PTR_ALIGN_UP(4, 3));

    assert_ptr_equal(0, PTR_ALIGN_UP_T((uint64_t *)0));
    assert_ptr_equal(8, PTR_ALIGN_UP_T((uint64_t *)1));
    assert_ptr_equal(8, PTR_ALIGN_UP_T((uint64_t *)2));
    assert_ptr_equal(8, PTR_ALIGN_UP_T((uint64_t *)4));
    assert_ptr_equal(8, PTR_ALIGN_UP_T((uint64_t *)7));
    assert_ptr_equal(8, PTR_ALIGN_UP_T((uint64_t *)8));
    assert_ptr_equal(16, PTR_ALIGN_UP_T((uint64_t *)9));
}

static void test_ptr_align_down(void **_state) {
    assert_ptr_equal(0, PTR_ALIGN_DOWN(0, 512));
    assert_ptr_equal(0, PTR_ALIGN_DOWN(1, 512));
    assert_ptr_equal(0, PTR_ALIGN_DOWN(2, 512));
    assert_ptr_equal(0, PTR_ALIGN_DOWN(511, 512));
    assert_ptr_equal(512, PTR_ALIGN_DOWN(512, 512));
    assert_ptr_equal(512, PTR_ALIGN_DOWN(513, 512));

    assert_ptr_equal(0, PTR_ALIGN_DOWN(0, 3));
    assert_ptr_equal(0, PTR_ALIGN_DOWN(1, 3));
    assert_ptr_equal(3, PTR_ALIGN_DOWN(3, 3));
    assert_ptr_equal(3, PTR_ALIGN_DOWN(4, 3));

    assert_ptr_equal(0, PTR_ALIGN_DOWN_T((uint64_t *)0));
    assert_ptr_equal(0, PTR_ALIGN_DOWN_T((uint64_t *)1));
    assert_ptr_equal(0, PTR_ALIGN_DOWN_T((uint64_t *)2));
    assert_ptr_equal(0, PTR_ALIGN_DOWN_T((uint64_t *)4));
    assert_ptr_equal(0, PTR_ALIGN_DOWN_T((uint64_t *)7));
    assert_ptr_equal(8, PTR_ALIGN_DOWN_T((uint64_t *)8));
    assert_ptr_equal(8, PTR_ALIGN_DOWN_T((uint64_t *)9));
}

static void test_snprintf() {
    char buf[8] = {0};

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s", "A big payload. Much larger than the receiving buffer can handle");
    assert_int_equal(0, buf[sizeof(buf) - 1]);

    if (0)
    {
        memset(buf, 0, sizeof(buf));
        strncpy(buf, "A big payload. Much larger than the receiving buffer can handle", sizeof(buf));
        assert_int_equal(0, buf[sizeof(buf) - 1]);
    }

    char buf0[0] = "";
    char *result = strncpy(buf0, "foobar", 1);
    assert_int_equal(buf0, result);
    assert_int_equal(0, sizeof(buf0));
}

int main(void) {
    const struct CMUnitTest html5_render_escaped[] = {
        // clang-format off
        cmocka_unit_test(test_ptr_is_aligned),
        cmocka_unit_test(test_ptr_align_up),
        cmocka_unit_test(test_ptr_align_down),
        cmocka_unit_test(test_snprintf),
        // clang-format on
    };
    return cmocka_run_group_tests(html5_render_escaped, NULL, NULL);
}
