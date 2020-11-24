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
    .rgb = {.red = 120, .green = 120, .blue = 130},
};
static const Pixel COLOR_EMPTY_SHADOW = {
    .rgb = {.red = 30, .green = 30, .blue = 40},
};
static const Pixel COLOR_WALL = {
    .rgb = {.red = 70, .green = 70, .blue = 80},
};
static const Pixel COLOR_WALL_SHADOW = {
    .rgb = {.red = 15, .green = 15, .blue = 25},
};

#endif
