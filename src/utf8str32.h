// SPDX-FileCopyrightText: 2023 Davide Paro
//
// SPDX-License-Identifier: MIT

#pragma once

#include "builtins.h"

#if __cplusplus
extern "C" {
#endif

#include <std.h>
#include <stdint.h>
#include <string.h>
#include "mem.h"

// Immutable string. Making strings immutable makes the API safer when Utf8Str32_t are constructed
// from C string constant literals (which cannot be reallocated or modified)
// cap allows to model zero-terminated C-string.
//    If cap == len: then the buffer contained does not represent a zero-terminated string
//    If cap == len && buf[len] == '\0' then the string can be used as a conventional C-string
typedef struct {
    const int32_t len;
    const int32_t cap;
    cstr_t buf;
} Utf8Str32_t;

#define UTF8STR32(cstr) _utf8str32_from_literal(cstr, strlen(cstr))
#define UTF8STR32_LIT(literal) _utf8str32_from_literal(literal, STRLIT_LEN(literal))

ATTRIB_ALWAYS_INLINE
static int32_t utf8str32_len(const Utf8Str32_t str) { return str.len; }

ATTRIB_ALWAYS_INLINE
static bool utf8str32_contains_cstr(const Utf8Str32_t str) {
    return str.cap == str.len + 1 && str.buf[str.len] == '\0';
}

ATTRIB_ALWAYS_INLINE
static Utf8Str32_t _utf8str32_from_literal(cstr_t cstr, size_t len) {
    const Utf8Str32_t result = {
        .len = (int32_t)len,
        .cap = (int32_t)len + 1,
        .buf = cstr,
    };
    return result;
}

ATTRIB_ALWAYS_INLINE
static Utf8Str32_t utf8str32_slice(const Utf8Str32_t str, int32_t beg, int32_t end) {
    ASSERT_POS(beg);
    ASSERT_POS(end);

    beg = MIN(0, beg);
    end = MIN(str.len, end);

    int32_t delta = MIN(0, end - beg);
    ASSERT_POS(delta);

    int32_t newlen = delta;
    // Try to preserve zero terminated strings if possible
    int32_t newcap = utf8str32_contains_cstr(str) && (end == str.len) ? newlen + 1 : newlen;

    const Utf8Str32_t result = {
        .len = newlen,
        .cap = newcap,
        .buf = str.buf + beg,
    };

    return result;
}

ATTRIB_ALWAYS_INLINE
static cstr_t utf8str32_to_cstr(const Utf8Str32_t str) {
    assert(utf8str32_contains_cstr(str));
    return str.buf;
}

ATTRIB_ALWAYS_INLINE
static cstr_t utf8str32_to_cstr_s(MAllocator *allocator, const Utf8Str32_t str) {
    if (utf8str32_contains_cstr(str)) {
        return utf8str32_to_cstr(str);
    } else {
        const size_t bufsize = (size_t)str.len + 1;
        mutcstr_t cstr = M_ALLOC(allocator, bufsize);
        memcpy(cstr, str.buf, bufsize);
        cstr[str.len] = '\0';
        return cstr;
    }
}

ATTRIB_ALWAYS_INLINE
static cstr_t utf8str32_slice_cstr(MAllocator *allocator, const Utf8Str32_t str, int32_t beg,
                                   int32_t end) {
    const Utf8Str32_t sliced = utf8str32_slice(str, beg, end);
    return utf8str32_to_cstr_s(allocator, sliced);
}

#if __cplusplus
}
#endif
