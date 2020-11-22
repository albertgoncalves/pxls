#ifndef __GEOM_H__
#define __GEOM_H__

#include "prelude.h"

#define SHADOW_APERTURE 0.5f
#define SHADOW_RADIUS   32

typedef enum {
    MASK_WALL = 1 << 0,
    MASK_VISIBLE = 1 << 1,
} Mask;

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

typedef struct {
    f32 slope_start;
    f32 slope_end;
    i16 x;
    i16 y;
    i16 loop_start;
    i8  x_sign;
    i8  y_sign;
} Octal;

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

static void init_mask(u8 mask[PX_HEIGHT][PX_WIDTH]) {
    for (u8 i = 0; i < HORIZONTAL_LINES_COUNT; ++i) {
        HorizontalLine line = HORIZONTAL_LINES[i];
        for (u8 x = line.x0; x < line.x1; ++x) {
            mask[line.y][x] |= MASK_WALL;
        }
    }
    for (u8 i = 0; i < VERTICAL_LINES_COUNT; ++i) {
        VerticalLine line = VERTICAL_LINES[i];
        for (u8 y = line.y0; y < line.y1; ++y) {
            mask[y][line.x] |= MASK_WALL;
        }
    }
}

static const i16 SHADOW_RADIUS_SQUARED = SHADOW_RADIUS * SHADOW_RADIUS;

static void set_mask_col_row(u8 mask[PX_HEIGHT][PX_WIDTH], Octal octal) {
    if (octal.slope_start < octal.slope_end) {
        return;
    }
    f32 next_start = octal.slope_start;
    for (i16 i = octal.loop_start; i <= SHADOW_RADIUS; ++i) {
        Bool prev_blocked = FALSE;
        Bool visible = FALSE;
        i16  y_delta = (i16)(i * octal.y_sign);
        i16  y_delta_squared = (i16)(y_delta * y_delta);
        i16  y = (i16)(octal.y + y_delta);
        for (i16 j = i; 0 <= j; --j) {
            f32 l_slope = (j - SHADOW_APERTURE) / (i + SHADOW_APERTURE);
            if (octal.slope_start < l_slope) {
                continue;
            }
            f32 r_slope = (j + SHADOW_APERTURE) / (i - SHADOW_APERTURE);
            if (r_slope < octal.slope_end) {
                break;
            }
            i16 x_delta = (i16)(j * octal.x_sign);
            i16 x = (i16)(octal.x + x_delta);
            if ((((x_delta * x_delta) + y_delta_squared) <
                 SHADOW_RADIUS_SQUARED) &&
                (0 <= x) && (x < PX_WIDTH) && (0 <= y) && (y < PX_HEIGHT))
            {
                mask[y][x] |= MASK_VISIBLE;
                visible = TRUE;
            }
            Bool blocked = (x < 0) || (y < 0) || (PX_WIDTH <= x) ||
                           (PX_HEIGHT <= y) || (mask[y][x] & MASK_WALL);
            if (prev_blocked) {
                if (blocked) {
                    next_start = l_slope;
                    continue;
                } else {
                    prev_blocked = FALSE;
                    octal.slope_start = next_start;
                }
            } else if (blocked && (i < SHADOW_RADIUS)) {
                {
                    Octal next_octal;
                    next_octal.slope_start = next_start;
                    next_octal.slope_end = r_slope;
                    next_octal.loop_start = (i16)(i + 1);
                    next_octal.x = octal.x, next_octal.y = octal.y,
                    next_octal.x_sign = octal.x_sign;
                    next_octal.y_sign = octal.y_sign;
                    set_mask_col_row(mask, next_octal);
                }
                prev_blocked = TRUE;
                next_start = l_slope;
            }
        }
        if (prev_blocked || (!visible)) {
            return;
        }
    }
}

static void set_mask_row_col(u8 mask[PX_HEIGHT][PX_WIDTH], Octal octal) {
    if (octal.slope_start < octal.slope_end) {
        return;
    }
    f32 next_start = octal.slope_start;
    for (i16 j = octal.loop_start; j <= SHADOW_RADIUS; ++j) {
        Bool prev_blocked = FALSE;
        Bool visible = FALSE;
        i16  x_delta = (i16)(j * octal.x_sign);
        i16  x_delta_squared = (i16)(x_delta * x_delta);
        i16  x = (i16)(octal.x + x_delta);
        for (i16 i = j; 0 <= i; --i) {
            f32 l_slope = (i - SHADOW_APERTURE) / (j + SHADOW_APERTURE);
            if (octal.slope_start < l_slope) {
                continue;
            }
            f32 r_slope = (i + SHADOW_APERTURE) / (j - SHADOW_APERTURE);
            if (r_slope < octal.slope_end) {
                break;
            }
            i16 y_delta = (i16)(i * octal.y_sign);
            i16 y = (i16)(octal.y + y_delta);
            if (((x_delta_squared + (y_delta * y_delta)) <
                 SHADOW_RADIUS_SQUARED) &&
                (0 <= x) && (x < PX_WIDTH) && (0 <= y) && (y < PX_HEIGHT))
            {
                mask[y][x] |= MASK_VISIBLE;
                visible = TRUE;
            }
            Bool blocked = (x < 0) || (y < 0) || (PX_WIDTH <= x) ||
                           (PX_HEIGHT <= y) || (mask[y][x] & MASK_WALL);
            if (prev_blocked) {
                if (blocked) {
                    next_start = l_slope;
                    continue;
                } else {
                    prev_blocked = FALSE;
                    octal.slope_start = next_start;
                }
            } else if (blocked && (j < SHADOW_RADIUS)) {
                {
                    Octal next_octal;
                    next_octal.slope_start = next_start;
                    next_octal.slope_end = r_slope;
                    next_octal.loop_start = (i16)(j + 1);
                    next_octal.x = octal.x, next_octal.y = octal.y,
                    next_octal.x_sign = octal.x_sign;
                    next_octal.y_sign = octal.y_sign;
                    set_mask_row_col(mask, next_octal);
                }
                prev_blocked = TRUE;
                next_start = l_slope;
            }
        }
        if (prev_blocked || (!visible)) {
            return;
        }
    }
}

#endif
