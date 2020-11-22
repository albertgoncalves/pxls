#ifndef __GEOM_H__
#define __GEOM_H__

#include "prelude.h"

typedef struct {
    u8 x0;
    u8 x1;
    u8 y;
} HorizontalLine;

typedef struct {
    u8 x;
    u8 y0;
    u8 y1;
} VerticalLine;

static const HorizontalLine HORIZONTAL_LINES[] = {
    {
        .x0 = 0,
        .x1 = 1,
        .y = 8,
    },
    {
        .x0 = 5,
        .x1 = 17,
        .y = 8,
    },
    {
        .x0 = 22,
        .x1 = 24,
        .y = 8,
    },
    {
        .x0 = 25,
        .x1 = 27,
        .y = 8,
    },
    {
        .x0 = 31,
        .x1 = 32,
        .y = 8,
    },
    {
        .x0 = 6,
        .x1 = 15,
        .y = 14,
    },
    {
        .x0 = 0,
        .x1 = 1,
        .y = 24,
    },
    {
        .x0 = 5,
        .x1 = 16,
        .y = 24,
    },
    {
        .x0 = 25,
        .x1 = 27,
        .y = 24,
    },
    {
        .x0 = 31,
        .x1 = 32,
        .y = 24,
    },
    {
        .x0 = 15,
        .x1 = 17,
        .y = 20,
    },
    {
        .x0 = 22,
        .x1 = 25,
        .y = 20,
    },
};

static const VerticalLine VERTICAL_LINES[] = {
    {
        .x = 6,
        .y0 = 0,
        .y1 = 1,
    },
    {
        .x = 6,
        .y0 = 6,
        .y1 = 8,
    },
    {
        .x = 24,
        .y0 = 0,
        .y1 = 1,
    },
    {
        .x = 24,
        .y0 = 6,
        .y1 = 16,
    },
    {
        .x = 24,
        .y0 = 21,
        .y1 = 32,
    },
    {
        .x = 6,
        .y0 = 9,
        .y1 = 14,
    },
    {
        .x = 15,
        .y0 = 9,
        .y1 = 15,
    },
    {
        .x = 6,
        .y0 = 25,
        .y1 = 32,
    },
    {
        .x = 15,
        .y0 = 21,
        .y1 = 27,
    },
    {
        .x = 15,
        .y0 = 31,
        .y1 = 32,
    },
};

static const u8 HORIZONTAL_LINES_COUNT =
    (u8)(sizeof(HORIZONTAL_LINES) / sizeof(HORIZONTAL_LINES[0]));

static const u8 VERTICAL_LINES_COUNT =
    (u8)(sizeof(VERTICAL_LINES) / sizeof(VERTICAL_LINES[0]));

#endif
