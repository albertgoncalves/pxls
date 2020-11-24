#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "prelude.h"

#define KEY_SENSITIVITY 0.0525f;

typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_COUNT,
    DIR_NONE,
} Direction;

typedef struct {
    f32 x;
    f32 y;
    f32 next_x;
    f32 next_y;
    u16 control[DIR_COUNT];
    // NOTE: It would take a lot of effort, but this *can* roll over.
    u16 control_counter;
} Player;

static Direction get_direction(Player* player) {
    u16       max = 0;
    Direction direction = DIR_NONE;
    for (u8 i = 0; i < DIR_COUNT; ++i) {
        if (max < player->control[i]) {
            max = player->control[i];
            direction = (Direction)i;
        }
    }
    return direction;
}

static void set_player_next_xy(Player* player) {
    switch (get_direction(player)) {
    case DIR_UP: {
        player->next_y -= KEY_SENSITIVITY;
        break;
    }
    case DIR_DOWN: {
        player->next_y += KEY_SENSITIVITY;
        break;
    }
    case DIR_LEFT: {
        player->next_x -= KEY_SENSITIVITY;
        break;
    }
    case DIR_RIGHT: {
        player->next_x += KEY_SENSITIVITY;
        break;
    }
    case DIR_COUNT:
    case DIR_NONE: {
        break;
    }
    }
}

static const f32 PX_WIDTH_MINUS_1 = PX_WIDTH - 1.0f;
static const f32 PX_HEIGHT_MINUS_1 = PX_HEIGHT - 1.0f;

static void update_player_position(u8      mask[PX_HEIGHT][PX_WIDTH],
                                   Player* player) {
    player->next_x = clamp_f32(player->next_x, 0.0f, PX_WIDTH_MINUS_1);
    player->next_y = clamp_f32(player->next_y, 0.0f, PX_HEIGHT_MINUS_1);
    if (mask[(u8)player->next_y][(u8)player->next_x] & MASK_WALL) {
        player->next_x = player->x;
        player->next_y = player->y;
    } else {
        player->x = player->next_x;
        player->y = player->next_y;
    }
}

#endif
