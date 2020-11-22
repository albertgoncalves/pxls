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
#define COLOR_PLAYER_GREEN 30
#define COLOR_PLAYER_BLUE  15

#define COLOR_VISIBLE_RED   120
#define COLOR_VISIBLE_GREEN 120
#define COLOR_VISIBLE_BLUE  140

#define COLOR_SHADOW_RED   14
#define COLOR_SHADOW_GREEN 14
#define COLOR_SHADOW_BLUE  18

#define COLOR_WALL_RED   42
#define COLOR_WALL_GREEN 42
#define COLOR_WALL_BLUE  48

static Pixel COLOR_PLAYER;
static Pixel COLOR_VISIBLE;
static Pixel COLOR_SHADOW;
static Pixel COLOR_WALL;

static void set_colors(void) {
    {
        COLOR_PLAYER.rgb.red = COLOR_PLAYER_RED;
        COLOR_PLAYER.rgb.green = COLOR_PLAYER_GREEN;
        COLOR_PLAYER.rgb.blue = COLOR_PLAYER_BLUE;
    }
    {
        COLOR_VISIBLE.rgb.red = COLOR_VISIBLE_RED;
        COLOR_VISIBLE.rgb.green = COLOR_VISIBLE_GREEN;
        COLOR_VISIBLE.rgb.blue = COLOR_VISIBLE_BLUE;
    }
    {
        COLOR_SHADOW.rgb.red = COLOR_SHADOW_RED;
        COLOR_SHADOW.rgb.green = COLOR_SHADOW_GREEN;
        COLOR_SHADOW.rgb.blue = COLOR_SHADOW_BLUE;
    }
    {
        COLOR_WALL.rgb.red = COLOR_WALL_RED;
        COLOR_WALL.rgb.green = COLOR_WALL_GREEN;
        COLOR_WALL.rgb.blue = COLOR_WALL_BLUE;
    }
}

#endif
