#ifndef __PRELUDE_H__
#define __PRELUDE_H__

#include <stdint.h>
#include <stdio.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int32_t i32;

typedef float f32;

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

static f32 clamp_f32(f32 x, f32 min, f32 max) {
    return x < min ? min : max < x ? max : x;
}

#endif
