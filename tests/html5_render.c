// SPDX-FileCopyrightText: 2024 Davide Paro
//
// SPDX-License-Identifier: MIT

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <memory.h>

#include "std.h"
#include "html5.h"

typedef struct TestState {
    char *input;
    char *expected;

    FILE *f;
    char *buf;
} TestState;


static int test_setup(void **_state) {
    size_t bufsize = 64 * 1024;
    TestState *state = *_state;
    state->buf = malloc(bufsize);
    if (state->buf) {
        memset(state->buf, 0, bufsize);
        state->f = fmemopen(state->buf, bufsize, "w");
    }

    if (state->buf && state->f) {
        return 0;
    } else {
        return 1;
    }
}

static int test_teardown(void **_state) {
    TestState *state = *_state;
    if (state->f) fclose(state->f);
    if (state->buf) free(state->buf);
    return 0;
}

static void test_html5_render_escaped(void **_state) {
    TestState *state = *_state;
    html5_render_escaped(state->f, state->input);
    fflush(state->f);
    assert_string_equal(state->buf, state->expected);
}

int main(int argc, char **argv) {
     const struct CMUnitTest html5_render_escaped[] = {
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = NULL, .expected = "" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "", .expected = "" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "foo", .expected = "foo" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "'", .expected = "&#x27;" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "\"", .expected = "&quot;" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "<", .expected = "&lt;" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = ">", .expected = "&gt;" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "<a href=\"ThreadSanitizer.html\">ThreadSanitizer</a>", .expected = "&lt;a href=&quot;ThreadSanitizer.html&quot;&gt;ThreadSanitizer&lt;/a&gt;" })  ),
     };

     return cmocka_run_group_tests(html5_render_escaped, NULL, NULL);

}


