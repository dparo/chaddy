// SPDX-FileCopyrightText: 2023 Davide Paro
//
// SPDX-License-Identifier: MIT

#pragma once

#include <hedley.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define BUILTIN_IS_CONSTANT(x) HEDLEY_IS_CONSTANT(x)

#define ATTRIB_ALWAYS_INLINE HEDLEY_ALWAYS_INLINE
#define ATTRIB_NO_RETURN HEDLEY_NO_RETURN
#define ATTRIB_NO_RETURN HEDLEY_NO_RETURN
#define ATTRIB_NO_DISCARD HEDLEY_WARN_UNUSED_RESULT
#define ATTRIB_NON_NULL HEDLEY_NON_NULL
#define ATTRIB_PRINTF_FORMAT(string_idx, first_to_check)                                           \
    HEDLEY_PRINTF_FORMAT(string_idx, first_to_check)
#define ATTRIB_PURE HEDLEY_PURE
#define ATTRIB_CONST HEDLEY_CONST

#ifdef __GNUC__
#define ATTRIB_ASSUME_ALIGNED(...) __attribute__((__assume_aligned__(__VA_ARGS__)))
#else
#endif

#ifdef __GNUC__
#define ATTRIB_WEAK __attribute__((__weak__))
#define ATTRIB_WEAK_ALIAS(alias) __attribute__((__weak__, __alias__(alias)))
#else
#define ATTRIB_WEAK
#define ATTRIB_WEAK_ALIAS(alias)
#endif

#ifdef __GNUC__
#define ATTRIB_CONSTRUCTOR(...) __attribute__((__constructor__(__VA_ARGS__)))
#define ATTRIB_DESTRUCTOR(...) __attribute__((__destructor__(__VA_ARGS__)))
#else
#define ATTRIB_CONSTRUCTOR(...)
#define ATTRIB_DESTRUCTOR(...)
#endif

#ifndef __cplusplus
#define ATTRIB_NO_THROW(...) HEDLEY_NO_THROW
#else
#define ATTRIB_NO_THROW(...) noexcept(__VA_ARGS__)
#endif

#if __cplusplus
extern "C" {
#endif
typedef char *mutcstr_t;
typedef char const *cstr_t;

typedef uintptr_t ptr_t;
typedef uint8_t byte_t;
typedef uint8_t byte8_t;
typedef int8_t i8_t;
typedef uint8_t u8_t;
typedef int16_t i16_t;
typedef uint16_t u16_t;
typedef int32_t i32_t;
typedef uint32_t u32_t;
typedef int64_t i64_t;
typedef uint64_t u64_t;

typedef float f32_t;
typedef double f64_t;

#define PTR(p) ((uintptr_t)(p))
#define UINTPTR(p) ((uintptr_t)(p))

#if __cplusplus
}
#endif
