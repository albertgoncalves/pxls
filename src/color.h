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

#define COLOR_EMPTY_RED   120
#define COLOR_EMPTY_GREEN 120
#define COLOR_EMPTY_BLUE  130

#define COLOR_EMPTY_SHADOW_RED   10
#define COLOR_EMPTY_SHADOW_GREEN 10
#define COLOR_EMPTY_SHADOW_BLUE  20

#define COLOR_WALL_RED   70
#define COLOR_WALL_GREEN 70
#define COLOR_WALL_BLUE  80

#define COLOR_WALL_SHADOW_RED   30
#define COLOR_WALL_SHADOW_GREEN 30
#define COLOR_WALL_SHADOW_BLUE  40

static Pixel COLOR_PLAYER;
static Pixel COLOR_EMPTY;
static Pixel COLOR_EMPTY_SHADOW;
static Pixel COLOR_WALL;
static Pixel COLOR_WALL_SHADOW;

static void set_colors(void) {
    {
        COLOR_PLAYER.rgb.red = COLOR_PLAYER_RED;
        COLOR_PLAYER.rgb.green = COLOR_PLAYER_GREEN;
        COLOR_PLAYER.rgb.blue = COLOR_PLAYER_BLUE;
    }
    {
        COLOR_EMPTY.rgb.red = COLOR_EMPTY_RED;
        COLOR_EMPTY.rgb.green = COLOR_EMPTY_GREEN;
        COLOR_EMPTY.rgb.blue = COLOR_EMPTY_BLUE;
    }
    {
        COLOR_EMPTY_SHADOW.rgb.red = COLOR_EMPTY_SHADOW_RED;
        COLOR_EMPTY_SHADOW.rgb.green = COLOR_EMPTY_SHADOW_GREEN;
        COLOR_EMPTY_SHADOW.rgb.blue = COLOR_EMPTY_SHADOW_BLUE;
    }
    {
        COLOR_WALL.rgb.red = COLOR_WALL_RED;
        COLOR_WALL.rgb.green = COLOR_WALL_GREEN;
        COLOR_WALL.rgb.blue = COLOR_WALL_BLUE;
    }
    {
        COLOR_WALL_SHADOW.rgb.red = COLOR_WALL_SHADOW_RED;
        COLOR_WALL_SHADOW.rgb.green = COLOR_WALL_SHADOW_GREEN;
        COLOR_WALL_SHADOW.rgb.blue = COLOR_WALL_SHADOW_BLUE;
    }
}

#endif
