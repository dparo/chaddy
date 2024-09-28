/*
 * SPDX-FileCopyrightText: 2024 Davide Paro <dparo@outlook.it>, et al.
 * SPDX-FileContributor: Davide Paro <dparo@outlook.it>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"
#include "stdlib.h"
#include <stdio.h>
#include <memory.h>

#include "html5.h"

typedef struct TestState {
    char *input;
    char *expected_output;

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
    if (state->f)
        fclose(state->f);
    if (state->buf)
        free(state->buf);
    return 0;
}

static void test_html5_render_escaped(void **_state) {
    TestState *state = *_state;
    HtmlRenderer r = {0};
    r.fstream = state->f;
    html5_render_escaped(&r, state->input);
    fflush(state->f);
    assert_string_equal(state->buf, state->expected_output);
}

int main(int argc, char **argv) {
    const struct CMUnitTest html5_render_escaped[] = {
        // clang-format off
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = NULL, .expected_output = "" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "", .expected_output = "" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "foo", .expected_output = "foo" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "'", .expected_output = "&#x27;" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "\"", .expected_output = "&quot;" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "<", .expected_output = "&lt;" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = ">", .expected_output = "&gt;" })  ),
        cmocka_unit_test_prestate_setup_teardown(test_html5_render_escaped, test_setup, test_teardown, & ((TestState) { .input = "<a href=\"ThreadSanitizer.html\">ThreadSanitizer</a>", .expected_output = "&lt;a href=&quot;ThreadSanitizer.html&quot;&gt;ThreadSanitizer&lt;/a&gt;" })  ),
        // clang-format on
    };

    return cmocka_run_group_tests(html5_render_escaped, NULL, NULL);
}
