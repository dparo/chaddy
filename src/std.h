// SPDX-FileCopyrightText: 2023 Davide Paro
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <builtins.h>
#include <assert.h>
#include <debugbreak.h>

#define STRINGIFY(X) #X

#if __cplusplus
extern "C" {
#endif

// typeof
#ifndef __cplusplus
#if __STDC_VERSION__ >= 202311L
// NOTE: typeof() are part of C23
#else
#ifdef __GNUC__
#define typeof(x) __typeof__(x)
#endif
#endif
#else
#define typeof(x) decltype(x)
#endif

#define CAST(T, expr) ((T)(expr))

#define CAST_SIZE (size_t)
#define CAST_SIZE_EXPR(expr) ((size_t)(expr))
#define PTRCAST_UINTPTR (uintptr_t)
#define PTRCAST_UINTPTR_EXPR(expr) ((uintptr_t)(expr))

#define PTRCAST_VOIDSTAR (void *)
#define PTRCAST_VOIDSTAR_EXPR(expr) ((void *)(expr))

#ifdef typeof
#define CAST_TYPEOF(t) (typeof(t))
#define PTRCAST_TYPEOF(t) (typeof(t))
#else
#define CAST_TYPEOF(t) ? ? ? ? ? ? ? ? ? ? ? ? ?
#define PTRCAST_TYPEOF(t) PTRCAST_VOIDSTAR
#endif

#define CAST_TYPEOF_EXPR(t, expr) (CAST_TYPEOF(t)(expr))
#define PTRCAST_TYPEOF_EXPR(t, expr) (PTRCAST_TYPEOF(t)(expr))

#ifndef __cplusplus
#ifdef __GNUC__
#define auto __auto_type
#endif
#else
// NOTE: auto is part of C++
#endif

// In C++, alignas(), alignof() are already part of the language spec
#if !defined alignas && !defined alignof
#ifndef __cplusplus
#if __STDC_VERSION__ >= 202311L
// NOTE: alignof(), alignas() are part of C23
#elif __STDC_VERSION__ >= 201112L
#define alignof(x) (_Alignof(x))
#define alignas(x) (_Alignas(x))
#else
#ifdef __GNUC__
#define alignof(x) (__alignof__(x))
#define alignas(x) __attribute__((__aligned__(x)))
#else
#include <stdalign.h>
#endif
#endif
#endif
#endif

#define ALIGNAS_EXPR(expr) (alignas(typeof(expr)))
#define ALIGNOF_EXPR(expr) (alignof(typeof(expr)))

#ifdef MAX
#undef MAX
#endif
#define MAX(a, b)                                                                                  \
    ({                                                                                             \
        auto _a = (a);                                                                             \
        auto _b = (b);                                                                             \
        _a > _b ? _a : _b;                                                                         \
    })

#ifdef MIN
#undef MIN
#endif
#define MIN(a, b)                                                                                  \
    ({                                                                                             \
        auto _a = (a);                                                                             \
        auto _b = (b);                                                                             \
        _a < _b ? _a : _b;                                                                         \
    })

#define SWAP(a, b)                                                                                 \
    ({                                                                                             \
        auto *_a = &(a);                                                                           \
        auto *_b = &(b);                                                                           \
        auto _t = _a;                                                                              \
        *_a = *_b;                                                                                 \
        *_b = _t;                                                                                  \
    })

#ifdef IS_POW2
#undef IS_POW2
#endif
#define IS_POW2(x)                                                                                 \
    ({                                                                                             \
        auto _x = (x);                                                                             \
        (((_x - 1) & _x) == 0);                                                                    \
    })

#define ASSERT_POW2(x) (assert(IS_POW2(x)))
#define ASSERT_ZERO(x) (assert((x) == CAST_TYPEOF(typeof(x)) 0))
#define ASSERT_NZERO(x) (assert((x) != (typeof(x))0))
#define ASSERT_POS(x) (assert((x) >= (typeof(x))0))
#define ASSERT_NEG(x) (assert((x) < (typeof(x))0))

#define PTR_ALIGN_UP(pointer, alignment)                                                           \
    ({                                                                                             \
        auto _p = PTRCAST_UINTPTR_EXPR(pointer);                                                   \
        auto _a = MAX((size_t)(alignment), 1U);                                                    \
        PTRCAST_TYPEOF_EXPR(pointer, (BUILTIN_IS_CONSTANT(alignment) && IS_POW2(_a))               \
                                         ? (_p + _a - 1) & ~(_a - 1)                               \
                                         : (_p + _a - 1) / _a * _a);                               \
    })

#define PTR_ALIGN_DOWN(pointer, alignment)                                                         \
    ({                                                                                             \
        auto _p = PTRCAST_UINTPTR_EXPR(pointer);                                                   \
        auto _a = MAX((size_t)(alignment), 1U);                                                    \
        PTRCAST_TYPEOF_EXPR(pointer, (BUILTIN_IS_CONSTANT(alignment) && IS_POW2(_a))               \
                                         ? _p & ~(_a - 1)                                          \
                                         : (_p / _a) * _a);                                        \
    })

#define PTR_ALIGN_UP_T(ptr) (PTR_ALIGN_UP((ptr), ALIGNOF_EXPR(*(ptr))))
#define PTR_ALIGN_DOWN_T(ptr) (PTR_ALIGN_DOWN((ptr), ALIGNOF_EXPR(*(ptr))))

#define PTR_IS_ALIGNED(ptr, alignment) _ptr_is_aligned(PTRCAST_UINTPTR_EXPR(ptr), alignment)

ATTRIB_ALWAYS_INLINE
static bool _ptr_is_aligned(uintptr_t ptr, size_t alignment) {
    return (uintptr_t)PTR_ALIGN_UP(ptr, alignment) == ptr;
}

ATTRIB_ALWAYS_INLINE
static uintptr_t ptr_roundup(uintptr_t ptr, size_t alignment) {
    alignment = MAX(1U, alignment);
    return (ptr + alignment - 1) / alignment * alignment;
}

ATTRIB_ALWAYS_INLINE
static uintptr_t ptr_roundup_pow2(uintptr_t ptr, size_t alignment) {
    alignment = MAX(1U, alignment);
    ASSERT_POW2(alignment);
    return (ptr + alignment - 1) & ~(alignment - 1);
}

ATTRIB_ALWAYS_INLINE
static uintptr_t ptr_rounddown(uintptr_t ptr, size_t alignment) {
    alignment = MAX(1U, alignment);
    return ptr / alignment * alignment;
}

ATTRIB_ALWAYS_INLINE
static uintptr_t ptr_rounddown_pow2(uintptr_t ptr, size_t alignment) {
    alignment = MAX(1U, alignment);
    ASSERT_POW2(alignment);
    return ptr & ~(alignment - 1);
}

ATTRIB_ALWAYS_INLINE
static size_t roundup(size_t val, size_t alignment) {
    alignment = MAX(1U, alignment);
    return (val + alignment - 1) / alignment * alignment;
}

ATTRIB_ALWAYS_INLINE
static size_t roundup_pow2(size_t val, size_t alignment) {
    alignment = MAX(1U, alignment);
    ASSERT_POW2(alignment);
    return (val + alignment - 1) & ~(alignment - 1);
}

ATTRIB_ALWAYS_INLINE
static size_t rounddown(size_t val, size_t alignment) {
    alignment = MAX(1U, alignment);
    return val / alignment * alignment;
}

ATTRIB_ALWAYS_INLINE
static size_t rounddown_pow2(size_t val, size_t alignment) {
    alignment = MAX(1U, alignment);
    ASSERT_POW2(alignment);
    return val & ~(alignment - 1);
}

ATTRIB_ALWAYS_INLINE
static bool is_pow2(size_t x) { return ((x - 1) & x) == 0; }

#if defined __GNUC__ || defined __GNUG__ || defined __clang__
#define ARRAY_LEN(arr)                                                                             \
    (sizeof(arr) / sizeof((arr)[0]) +                                                              \
     sizeof(typeof(int[1 - 2 * !!__builtin_types_compatible_p(typeof(arr), typeof(&(arr)[0]))])) * \
         0)
#else
#define ARRAY_LEN(A)                                                                               \
    ((sizeof(A) / sizeof((A)[0])) /                                                                \
     ((size_t) !(sizeof(A) % sizeof((A)[0])))) /* Make sure that the sizeof(A) is a multiple of    \
                                                  sizeof(A[0]), if this does not hold divide by    \
                                                  zero to trigger a warning */
#endif

#define STRLIT_LEN(cstr_literal) (ARRAY_LEN(cstr_literal) - 1)

#if __cplusplus
}
#endif
