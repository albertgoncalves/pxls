#ifndef __COLOR_H__
#define __COLOR_H__

#include "prelude.h"

typedef struct {
    u8 red;
    u8 green;
    u8 blue;
} Rgb;

typedef union {
    u32 pack;
    Rgb rgb;
} Pixel;

static const Pixel COLOR_PLAYER = {
    .rgb = {.red = 220, .green = 30, .blue = 15},
};
static const Pixel COLOR_EMPTY = {
    .rgb = {.red = 10, .green = 15, .blue = 30},
};
static const Pixel COLOR_WALL = {
    .rgb = {.red = 30, .green = 35, .blue = 50},
};
static const Pixel COLOR_LIGHT = {
    .rgb = {.red = 50, .green = 50, .blue = 55},
};

#endif
