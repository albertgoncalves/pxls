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

#define COLOR_PLAYER_RED   220
#define COLOR_PLAYER_GREEN 60
#define COLOR_PLAYER_BLUE  80

#define COLOR_EMPTY_RED   58
#define COLOR_EMPTY_GREEN 58
#define COLOR_EMPTY_BLUE  62

#define COLOR_SHADOW_RED   12
#define COLOR_SHADOW_GREEN 12
#define COLOR_SHADOW_BLUE  12

#define COLOR_WALL_RED   32
#define COLOR_WALL_GREEN 32
#define COLOR_WALL_BLUE  38

static Pixel COLOR_PLAYER;
static Pixel COLOR_EMPTY;
static Pixel COLOR_SHADOW;
static Pixel COLOR_WALL;

static void set_colors(void) {
    COLOR_PLAYER.rgb.red = COLOR_PLAYER_RED;
    COLOR_PLAYER.rgb.green = COLOR_PLAYER_GREEN;
    COLOR_PLAYER.rgb.blue = COLOR_PLAYER_BLUE;
    COLOR_EMPTY.rgb.red = COLOR_EMPTY_RED;
    COLOR_EMPTY.rgb.green = COLOR_EMPTY_GREEN;
    COLOR_EMPTY.rgb.blue = COLOR_EMPTY_BLUE;
    COLOR_SHADOW.rgb.red = COLOR_SHADOW_RED;
    COLOR_SHADOW.rgb.green = COLOR_SHADOW_GREEN;
    COLOR_SHADOW.rgb.blue = COLOR_SHADOW_BLUE;
    COLOR_WALL.rgb.red = COLOR_WALL_RED;
    COLOR_WALL.rgb.green = COLOR_WALL_GREEN;
    COLOR_WALL.rgb.blue = COLOR_WALL_BLUE;
}

#endif
