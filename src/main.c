#include "color.h"
#include "geom.h"
#include "player.h"

#include <SDL2/SDL.h>

// NOTE: See `https://benedicthenshaw.com/soft_render_sdl2.html`.

typedef struct {
    u32 start;
    u32 end;
    u32 prev;
    u32 delta;
    u32 fps_start;
    u16 update_count;
    u8  fps_count;
} Frame;

typedef struct {
    Pixel  buffer[PX_HEIGHT][PX_WIDTH];
    u8     mask[PX_HEIGHT][PX_WIDTH];
    Player player;
    Frame  frame;
    Bool   dead;
} Memory;

#define PLAYER_SHADOW_RADIUS 32

#define FRAME_UPDATE_COUNT   8
#define FRAME_DEBUG_INTERVAL 30

#define MILLISECONDS 1000.0f

static const f32 FRAME_DURATION = (1.0f / 60.0f) * MILLISECONDS;

static void set_input(Player* player, Bool* dead) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: {
            *dead = TRUE;
            return;
        }
        case SDL_KEYDOWN: {
            if (event.key.repeat) {
                continue;
            }
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: {
                *dead = TRUE;
                return;
            }
            case SDLK_w:
            case SDLK_i: {
                player->control[DIR_UP] = ++player->control_counter;
                break;
            }
            case SDLK_s:
            case SDLK_k: {
                player->control[DIR_DOWN] = ++player->control_counter;
                break;
            }
            case SDLK_a:
            case SDLK_j: {
                player->control[DIR_LEFT] = ++player->control_counter;
                break;
            }
            case SDLK_d:
            case SDLK_l: {
                player->control[DIR_RIGHT] = ++player->control_counter;
                break;
            }
            }
            break;
        }
        case SDL_KEYUP: {
            switch (event.key.keysym.sym) {
            case SDLK_w:
            case SDLK_i: {
                player->control[DIR_UP] = 0;
                break;
            }
            case SDLK_s:
            case SDLK_k: {
                player->control[DIR_DOWN] = 0;
                break;
            }
            case SDLK_a:
            case SDLK_j: {
                player->control[DIR_LEFT] = 0;
                break;
            }
            case SDLK_d:
            case SDLK_l: {
                player->control[DIR_RIGHT] = 0;
                break;
            }
            }
            break;
        }
        }
        if (*((u64*)&player->control[0]) == 0) {
            player->control_counter = 0;
        }
    }
}

static const u32 FRAME_UPDATE_STEP =
    (u32)(FRAME_DURATION / (f32)FRAME_UPDATE_COUNT);

static void update_frame(u8      mask[PX_HEIGHT][PX_WIDTH],
                         Player* player,
                         Frame*  frame) {
    frame->delta += frame->start - frame->prev;
    while (FRAME_UPDATE_STEP < frame->delta) {
        set_player_next_xy(player);
        update_player_position(mask, player);
        frame->delta -= FRAME_UPDATE_STEP;
        ++frame->update_count;
    }
    frame->prev = frame->start;
}

static u8 MASK_RESET[16] = {
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
    (u8)~MASK_PLAYER,
};

static const i16 PLAYER_SHADOW_RADIUS_SQUARED =
    PLAYER_SHADOW_RADIUS * PLAYER_SHADOW_RADIUS;

static void set_mask(u8 mask[PX_HEIGHT][PX_WIDTH], Player* player) {
    {
        // NOTE: `PX_WIDTH_BY_HEIGHT` *must* to be divisible by 16.
        u8* pointer = &mask[0][0];
        for (u16 i = 0; i < PX_WIDTH_BY_HEIGHT; i = (u16)(i + 16)) {
            _mm_store_si128((Simd4i32*)&pointer[i],
                            _mm_and_si128(*(Simd4i32*)&pointer[i],
                                          *(Simd4i32*)&MASK_RESET[0]));
        }
    }
    i16 x = (i16)player->x;
    i16 y = (i16)player->y;
    mask[y][x] &= MASK_PLAYER;
    Octal octal = {
        .slope_start = 1.0f,
        .slope_end = 0.0f,
        .x = x,
        .y = y,
        .loop_start = 1,
        .radius = PLAYER_SHADOW_RADIUS,
        .radius_squared = PLAYER_SHADOW_RADIUS_SQUARED,
        .mask = MASK_PLAYER,
    };
    {
        octal.x_sign = 1;
        octal.y_sign = 1;
        set_mask_col_row(mask, octal);
        set_mask_row_col(mask, octal);
    }
    {
        octal.x_sign = 1;
        octal.y_sign = -1;
        set_mask_col_row(mask, octal);
        set_mask_row_col(mask, octal);
    }
    {
        octal.x_sign = -1;
        octal.y_sign = -1;
        set_mask_col_row(mask, octal);
        set_mask_row_col(mask, octal);
    }
    {
        octal.x_sign = -1;
        octal.y_sign = 1;
        set_mask_col_row(mask, octal);
        set_mask_row_col(mask, octal);
    }
}

static void set_buffer(Pixel   buffer[PX_HEIGHT][PX_WIDTH],
                       u8      mask[PX_HEIGHT][PX_WIDTH],
                       Player* player) {
    for (u8 i = 0; i < PX_HEIGHT; ++i) {
        for (u8 j = 0; j < PX_WIDTH; ++j) {
            if (mask[i][j] & MASK_WALL) {
                buffer[i][j].pack = COLOR_WALL.pack;
            } else {
                buffer[i][j].pack = COLOR_EMPTY.pack;
            }
            if (mask[i][j] & MASK_PLAYER) {
                buffer[i][j].rgb.red =
                    (u8)(buffer[i][j].rgb.red + COLOR_LIGHT.rgb.red);
                buffer[i][j].rgb.green =
                    (u8)(buffer[i][j].rgb.green + COLOR_LIGHT.rgb.green);
                buffer[i][j].rgb.blue =
                    (u8)(buffer[i][j].rgb.blue + COLOR_LIGHT.rgb.blue);
            }
        }
    }
    buffer[(u8)player->y][(u8)player->x].pack = COLOR_PLAYER.pack;
}

static void set_debug(Player* player, Frame* frame) {
    frame->end = SDL_GetTicks();
    f32 elapsed = (f32)(frame->end - frame->start);
    if (elapsed < FRAME_DURATION) {
        SDL_Delay((u32)(FRAME_DURATION - elapsed));
    }
    if (FRAME_DEBUG_INTERVAL <= ++frame->fps_count) {
        printf("\033[8A"
               "frames  / sec.       :%6.2f\n"
               "updates / frame      :%6.2f\n"
               "player.x             :%6.2f\n"
               "player.y             :%6.2f\n"
               "player.control.up    :%6hu\n"
               "player.control.down  :%6hu\n"
               "player.control.left  :%6hu\n"
               "player.control.right :%6hu\n",
               (f32)frame->fps_count / (f32)(frame->end - frame->fps_start) *
                   MILLISECONDS,
               (f32)frame->update_count / (f32)FRAME_DEBUG_INTERVAL,
               player->x,
               player->y,
               player->control[DIR_UP],
               player->control[DIR_DOWN],
               player->control[DIR_LEFT],
               player->control[DIR_RIGHT]);
        frame->fps_start = frame->start;
        frame->fps_count = 0;
        frame->update_count = 0;
    }
}

static const u32 TEXTURE_WIDTH = PX_WIDTH * sizeof(Pixel);

static void loop(SDL_Renderer* renderer,
                 SDL_Texture*  texture,
                 Memory*       memory) {
    Player* player = &memory->player;
    player->x = PX_WIDTH / 2.0f;
    player->y = PX_HEIGHT / 2.0f;
    player->next_x = player->x;
    player->next_y = player->y;
    Frame* frame = &memory->frame;
    Bool*  dead = &memory->dead;
    Pixel* pointer = &memory->buffer[0][0];
    init_mask(memory->mask);
    printf("\n\n\n\n\n\n\n\n");
    for (;;) {
        frame->start = SDL_GetTicks();
        set_input(player, dead);
        if (memory->dead) {
            return;
        }
        update_frame(memory->mask, player, frame);
        set_mask(memory->mask, player);
        set_buffer(memory->buffer, memory->mask, player);
        if (SDL_RenderClear(renderer) < 0) {
            ERROR("SDL_RenderClear(...) < 0");
        }
        if (SDL_UpdateTexture(texture, NULL, pointer, TEXTURE_WIDTH) < 0) {
            ERROR("SDL_UpdateTexture(...) < 0");
        }
        if (SDL_RenderCopy(renderer, texture, NULL, NULL) < 0) {
            ERROR("SDL_RenderCopy(...) < 0");
        }
        SDL_RenderPresent(renderer);
        set_debug(player, frame);
    }
}

static const u32 WINDOW_WIDTH = PX_WIDTH * PX_SCALE;
static const u32 WINDOW_HEIGHT = PX_HEIGHT * PX_SCALE;

i32 main(void) {
    printf("sizeof(Frame)          : %zu\n"
           "sizeof(Rgb)            : %zu\n"
           "sizeof(Pixel)          : %zu\n"
           "sizeof(Player)         : %zu\n"
           "sizeof(HorizontalLine) : %zu\n"
           "sizeof(VerticalLine)   : %zu\n"
           "sizeof(Octal)          : %zu\n"
           "sizeof(Memory)         : %zu\n\n",
           sizeof(Frame),
           sizeof(Rgb),
           sizeof(Pixel),
           sizeof(Player),
           sizeof(HorizontalLine),
           sizeof(VerticalLine),
           sizeof(Octal),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    if (!memory) {
        ERROR("!memory");
    }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        ERROR("SDL_Init(...) < 0");
    }
    SDL_Window* window = SDL_CreateWindow("float",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          SDL_WINDOW_RESIZABLE);
    if (!window) {
        ERROR("!window");
    }
    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        ERROR("!renderer");
    }
    SDL_SetWindowMinimumSize(window, PX_WIDTH, PX_HEIGHT);
    if (SDL_RenderSetLogicalSize(renderer, PX_WIDTH, PX_HEIGHT) < 0) {
        ERROR("SDL_RenderSetLogicalSize(...) < 0");
    }
    if (SDL_RenderSetIntegerScale(renderer, 1) < 0) {
        ERROR("SDL_RenderSetIntegerScale(...) < 0");
    }
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_BGR888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             PX_WIDTH,
                                             PX_HEIGHT);
    if (!texture) {
        ERROR("!texture");
    }
    loop(renderer, texture, memory);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(memory);
    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
