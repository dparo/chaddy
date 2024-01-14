// SPDX-FileCopyrightText: 2023 Davide Paro
//
// SPDX-License-Identifier: MIT

#pragma once

#include "std.h"
#include "mem.h"
#include "builtins.h"

// Example usage:
//      auto stack = STACK_MAKE(int32_t)(allocator);
//      STACK_PUSH(stack, 10);

typedef struct stack_header {
    int32_t len, cap;
    // TODO
    const MAllocator *allocator;
} stack_header_t;

#define STACK(T) stack_type_t_##T
#define STACK_MAKE(T) (stack_make_##T)
#define STACK_DEFINE(T)                                                                            \
    struct stack_##T {                                                                             \
        stack_header_t hdr;                                                                        \
        T *inner;                                                                                  \
    } static inline stack_make_##T(const MAllocator *allocator) {                                  \
        struct stack_##T result = {0};                                                             \
        result.hdr.allocator = allocator;                                                          \
        return result;                                                                             \
    }                                                                                              \
    typedef struct stack_##T stack_type_t_##T

// TODO
#define STACK_RESERVE(stack, cap) ({})

// TODO
#define STACK_PUSH(stack, elem)                                                                    \
    ({                                                                                             \
        auto _stack = (stack);                                                                     \
        auto _elem = (elem);                                                                       \
        stack_header_t _hdr = _stack.hdr;                                                          \
        _stack.inner[_hdr.len++] = _elem;                                                          \
    })

#define STACK_NTH(stack, n)                                                                        \
    ({                                                                                             \
        auto _stack = (stack);                                                                     \
        int32_t _n = (n);                                                                          \
        stack_header_t _hdr = _stack.hdr;                                                          \
        assert(_n > 0 && _n < _hdr.len);                                                           \
        _stack.inner[_n];                                                                          \
    })

#define STACK_NTH_REL(stack, offset)                                                               \
    ({                                                                                             \
        auto _stack = (stack);                                                                     \
        int32_t _offset = (offset);                                                                \
        stack_header_t _hdr = _stack.hdr;                                                          \
        int32_t _n = _stack.hdr.len + _offset;                                                     \
        assert(_n > 0 && _n < _hdr.len);                                                           \
        _stack.inner[_n];                                                                          \
    })

STACK_DEFINE(int);
STACK_DEFINE(int32_t);
STACK_DEFINE(uint32_t);

STACK_DEFINE(float);
STACK_DEFINE(double);
