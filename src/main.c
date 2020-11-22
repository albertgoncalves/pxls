#include "prelude.h"

#include <SDL2/SDL.h>

// NOTE: See `https://benedicthenshaw.com/soft_render_sdl2.html`.

#define PX_WIDTH  32
#define PX_HEIGHT 32

#define PX_SCALE 32

static const u32 WINDOW_WIDTH = PX_WIDTH * PX_SCALE;
static const u32 WINDOW_HEIGHT = PX_HEIGHT * PX_SCALE;

static const f32 PX_WIDTH_MINUS_1 = PX_WIDTH - 1.0f;
static const f32 PX_HEIGHT_MINUS_1 = PX_HEIGHT - 1.0f;

#define FRAME_UPDATE_COUNT   8
#define FRAME_DEBUG_INTERVAL 30

#define MILLISECONDS 1000.0f

static const f32 FRAME_DURATION = (1.0f / 60.0f) * MILLISECONDS;
static const u32 FRAME_UPDATE_STEP =
    (u32)(FRAME_DURATION / (f32)FRAME_UPDATE_COUNT);

#define KEY_SENSITIVITY 0.065;

#define COLOR_PLAYER_RED   220
#define COLOR_PLAYER_GREEN 60
#define COLOR_PLAYER_BLUE  80

#define COLOR_EMPTY_RED   58
#define COLOR_EMPTY_GREEN 58
#define COLOR_EMPTY_BLUE  62

#define COLOR_WALL_RED   32
#define COLOR_WALL_GREEN 32
#define COLOR_WALL_BLUE  38

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
    u8 red;
    u8 green;
    u8 blue;
} Rgb;

typedef union {
    u32 pack;
    Rgb rgb;
} Pixel;

static Pixel COLOR_EMPTY;
static Pixel COLOR_PLAYER;
static Pixel COLOR_WALL;

typedef enum {
    STATE_ALIVE = 0,
    STATE_DEAD = 1 << 0,
} State;

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
        .x = 14,
        .y0 = 9,
        .y1 = 14,
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

typedef struct {
    Pixel buffer[PX_HEIGHT][PX_WIDTH];
} Memory;

static f32 clamp_f32(f32 x, f32 min, f32 max) {
    return x < min ? min : max < x ? max : x;
}

static void set_buffer(Player* player, Pixel buffer[PX_HEIGHT][PX_WIDTH]) {
    for (u32 i = 0; i < PX_HEIGHT; ++i) {
        for (u32 j = 0; j < PX_WIDTH; ++j) {
            buffer[i][j].pack = COLOR_EMPTY.pack;
        }
    }
    for (u8 i = 0; i < HORIZONTAL_LINES_COUNT; ++i) {
        HorizontalLine line = HORIZONTAL_LINES[i];
        for (u8 x = line.x0; x < line.x1; ++x) {
            buffer[line.y][x].pack = COLOR_WALL.pack;
        }
    }
    for (u8 i = 0; i < VERTICAL_LINES_COUNT; ++i) {
        VerticalLine line = VERTICAL_LINES[i];
        for (u8 y = line.y0; y < line.y1; ++y) {
            buffer[y][line.x].pack = COLOR_WALL.pack;
        }
    }
    buffer[(u8)player->y][(u8)player->x].pack = COLOR_PLAYER.pack;
}

static void set_state(u8* state, Player* player) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: {
            *state |= STATE_DEAD;
            return;
        }
        case SDL_KEYDOWN: {
            if (event.key.repeat) {
                continue;
            }
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: {
                *state |= STATE_DEAD;
                return;
            }
            case SDLK_w: {
                player->control[DIR_UP] = ++player->control_counter;
                break;
            }
            case SDLK_s: {
                player->control[DIR_DOWN] = ++player->control_counter;
                break;
            }
            case SDLK_a: {
                player->control[DIR_LEFT] = ++player->control_counter;
                break;
            }
            case SDLK_d: {
                player->control[DIR_RIGHT] = ++player->control_counter;
                break;
            }
            }
            break;
        }
        case SDL_KEYUP: {
            switch (event.key.keysym.sym) {
            case SDLK_w: {
                player->control[DIR_UP] = 0;
                break;
            }
            case SDLK_s: {
                player->control[DIR_DOWN] = 0;
                break;
            }
            case SDLK_a: {
                player->control[DIR_LEFT] = 0;
                break;
            }
            case SDLK_d: {
                player->control[DIR_RIGHT] = 0;
                break;
            }
            }
            break;
        }
        }
        if ((player->control[0] + player->control[1] + player->control[2] +
             player->control[3]) == 0)
        {
            player->control_counter = 0;
        }
    }
}

static void update_player_position(Player* player,
                                   Pixel   buffer[PX_HEIGHT][PX_WIDTH]) {
    player->next_x = clamp_f32(player->next_x, 0.0f, PX_WIDTH_MINUS_1);
    player->next_y = clamp_f32(player->next_y, 0.0f, PX_HEIGHT_MINUS_1);
    Pixel next_pixel = buffer[(u8)player->next_y][(u8)player->next_x];
    if (next_pixel.pack != COLOR_WALL.pack) {
        player->x = player->next_x;
        player->y = player->next_y;
        return;
    }
    player->next_x = player->x;
    player->next_y = player->y;
}

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

static void update_frame(Frame*  frame,
                         Player* player,
                         Pixel   buffer[PX_HEIGHT][PX_WIDTH]) {
    frame->delta += frame->start - frame->prev;
    while (FRAME_UPDATE_STEP < frame->delta) {
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
        update_player_position(player, buffer);
        frame->delta -= FRAME_UPDATE_STEP;
        ++frame->update_count;
    }
    frame->prev = frame->start;
}

static void debug_frame(Frame* frame, Player* player) {
    frame->end = SDL_GetTicks();
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
                 Pixel         buffer[PX_HEIGHT][PX_WIDTH]) {
    Frame  frame = {0};
    u8     state = 0;
    Player player = {0};
    player.x = PX_WIDTH / 2.0f;
    player.y = PX_HEIGHT / 2.0f;
    player.next_x = player.x;
    player.next_y = player.y;
    printf("\n\n\n\n\n\n\n\n");
    for (;;) {
        frame.start = SDL_GetTicks();
        set_state(&state, &player);
        if (state & STATE_DEAD) {
            return;
        }
        update_frame(&frame, &player, buffer);
        set_buffer(&player, buffer);
        {
            if (SDL_RenderClear(renderer) < 0) {
                ERROR("SDL_RenderClear(...) < 0");
            }
            if (SDL_UpdateTexture(texture, NULL, buffer, TEXTURE_WIDTH) < 0) {
                ERROR("SDL_UpdateTexture(...) < 0");
            }
            if (SDL_RenderCopy(renderer, texture, NULL, NULL) < 0) {
                ERROR("SDL_RenderCopy(...) < 0");
            }
            SDL_RenderPresent(renderer);
        }
        debug_frame(&frame, &player);
    }
}

i32 main(void) {
    printf("sizeof(Frame)     : %zu\n"
           "sizeof(Rgb)       : %zu\n"
           "sizeof(Pixel)     : %zu\n"
           "sizeof(State)     : %zu\n"
           "sizeof(Direction) : %zu\n"
           "sizeof(Player)    : %zu\n"
           "sizeof(Memory)    : %zu\n\n",
           sizeof(Frame),
           sizeof(Rgb),
           sizeof(Pixel),
           sizeof(State),
           sizeof(Direction),
           sizeof(Player),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    if (!memory) {
        ERROR("!memory");
    }
    COLOR_EMPTY.rgb.red = COLOR_EMPTY_RED;
    COLOR_EMPTY.rgb.green = COLOR_EMPTY_GREEN;
    COLOR_EMPTY.rgb.blue = COLOR_EMPTY_BLUE;
    COLOR_PLAYER.rgb.red = COLOR_PLAYER_RED;
    COLOR_PLAYER.rgb.green = COLOR_PLAYER_GREEN;
    COLOR_PLAYER.rgb.blue = COLOR_PLAYER_BLUE;
    COLOR_WALL.rgb.red = COLOR_WALL_RED;
    COLOR_WALL.rgb.green = COLOR_WALL_GREEN;
    COLOR_WALL.rgb.blue = COLOR_WALL_BLUE;
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
    loop(renderer, texture, memory->buffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(memory);
    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
