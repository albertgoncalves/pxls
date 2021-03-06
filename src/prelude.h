#ifndef __PRELUDE_H__
#define __PRELUDE_H__

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

typedef float f32;

typedef __m128i Simd4i32;

typedef enum {
    FALSE = 0,
    TRUE = 1,
} Bool;

#define ERROR(x)                     \
    {                                \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                x);                  \
        exit(EXIT_FAILURE);          \
    }

#define PX_WIDTH  32
#define PX_HEIGHT 32

#define PX_SCALE 24

static const u16 PX_WIDTH_BY_HEIGHT = PX_WIDTH * PX_HEIGHT;

static f32 clamp_f32(f32 x, f32 min, f32 max) {
    return x < min ? min : max < x ? max : x;
}

#endif
