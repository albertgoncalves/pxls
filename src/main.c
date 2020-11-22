#include "prelude.h"

#include <SDL2/SDL.h>

// NOTE: See `https://benedicthenshaw.com/soft_render_sdl2.html`.

#define PX_WIDTH  64
#define PX_HEIGHT 48

#define PX_SCALE 16

static const u32 WINDOW_WIDTH = PX_WIDTH * PX_SCALE;
static const u32 WINDOW_HEIGHT = PX_HEIGHT * PX_SCALE;

static f32 PLAYER_X = PX_WIDTH / 2.0f;
static f32 PLAYER_Y = PX_HEIGHT / 2.0f;

static f32 NEXT_PLAYER_X = PX_WIDTH / 2.0f;
static f32 NEXT_PLAYER_Y = PX_HEIGHT / 2.0f;

static const f32 PX_WIDTH_MINUS_1 = PX_WIDTH - 1.0f;
static const f32 PX_HEIGHT_MINUS_1 = PX_HEIGHT - 1.0f;

#define FRAME_UPDATE_COUNT   8
#define FRAME_DEBUG_INTERVAL 30

#define MILLISECONDS 1000.0f

static const f32 FRAME_DURATION = (1.0f / 60.0f) * MILLISECONDS;
static const u32 FRAME_UPDATE_STEP =
    (u32)(FRAME_DURATION / (f32)FRAME_UPDATE_COUNT);

#define KEY_SENSITIVITY 0.0825;

#define COLOR_BACKGROUND_RED   32
#define COLOR_BACKGROUND_GREEN 32
#define COLOR_BACKGROUND_BLUE  32

#define COLOR_PLAYER_RED   220
#define COLOR_PLAYER_GREEN 60
#define COLOR_PLAYER_BLUE  80

#define COLOR_WALL_RED   72
#define COLOR_WALL_GREEN 72
#define COLOR_WALL_BLUE  72

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
    u8 _;
} Pixel;

typedef enum {
    INPUT_DEAD = 1 << 0,
    INPUT_UP = 1 << 1,
    INPUT_DOWN = 1 << 2,
    INPUT_LEFT = 1 << 3,
    INPUT_RIGHT = 1 << 4,
} Input;

typedef struct {
    Pixel buffer[PX_HEIGHT][PX_WIDTH];
} Memory;

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
        .x0 = 10,
        .x1 = 51,
        .y = 40,
    },
};

static const VerticalLine VERTICAL_LINES[] = {
    {
        .x = 10,
        .y0 = 10,
        .y1 = 40,
    },
};

static const u8 HORIZONTAL_LINES_COUNT =
    (u8)(sizeof(HORIZONTAL_LINES) / sizeof(HORIZONTAL_LINES[0]));

static const u8 VERTICAL_LINES_COUNT =
    (u8)(sizeof(VERTICAL_LINES) / sizeof(VERTICAL_LINES[0]));

static const Pixel COLOR_BACKGROUND = {
    .red = COLOR_BACKGROUND_RED,
    .green = COLOR_BACKGROUND_GREEN,
    .blue = COLOR_BACKGROUND_BLUE,
};

static const Pixel COLOR_PLAYER = {
    .red = COLOR_PLAYER_RED,
    .green = COLOR_PLAYER_GREEN,
    .blue = COLOR_PLAYER_BLUE,
};

static const Pixel COLOR_WALL = {
    .red = COLOR_WALL_RED,
    .green = COLOR_WALL_GREEN,
    .blue = COLOR_WALL_BLUE,
};

static f32 clamp_f32(f32 x, f32 min, f32 max) {
    return x < min ? min : max < x ? max : x;
}

static void set_buffer(Pixel buffer[PX_HEIGHT][PX_WIDTH]) {
    for (u32 i = 0; i < PX_HEIGHT; ++i) {
        for (u32 j = 0; j < PX_WIDTH; ++j) {
            buffer[i][j] = COLOR_BACKGROUND;
        }
    }
    for (u8 i = 0; i < HORIZONTAL_LINES_COUNT; ++i) {
        HorizontalLine line = HORIZONTAL_LINES[i];
        for (u8 x = line.x0; x < line.x1; ++x) {
            buffer[line.y][x] = COLOR_WALL;
        }
    }
    for (u8 i = 0; i < VERTICAL_LINES_COUNT; ++i) {
        VerticalLine line = VERTICAL_LINES[i];
        for (u8 y = line.y0; y < line.y1; ++y) {
            buffer[y][line.x] = COLOR_WALL;
        }
    }
    buffer[(u8)PLAYER_Y][(u8)PLAYER_X] = COLOR_PLAYER;
}

static void set_input(u8* input) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: {
            *input |= INPUT_DEAD;
            return;
        }
        case SDL_KEYDOWN: {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: {
                *input |= INPUT_DEAD;
                return;
            }
            case SDLK_w: {
                *input |= INPUT_UP;
                break;
            }
            case SDLK_s: {
                *input |= INPUT_DOWN;
                break;
            }
            case SDLK_a: {
                *input |= INPUT_LEFT;
                break;
            }
            case SDLK_d: {
                *input |= INPUT_RIGHT;
                break;
            }
            }
            break;
        }
        case SDL_KEYUP: {
            switch (event.key.keysym.sym) {
            case SDLK_w: {
                *input &= (u8)(~INPUT_UP);
                break;
            }
            case SDLK_s: {
                *input &= (u8)(~INPUT_DOWN);
                break;
            }
            case SDLK_a: {
                *input &= (u8)(~INPUT_LEFT);
                break;
            }
            case SDLK_d: {
                *input &= (u8)(~INPUT_RIGHT);
                break;
            }
            }
            break;
        }
        }
    }
}

static void update_player_position(Pixel buffer[PX_HEIGHT][PX_WIDTH]) {
    NEXT_PLAYER_X = clamp_f32(NEXT_PLAYER_X, 0.0f, PX_WIDTH_MINUS_1);
    NEXT_PLAYER_Y = clamp_f32(NEXT_PLAYER_Y, 0.0f, PX_HEIGHT_MINUS_1);
    {
        Pixel next_pixel = buffer[(u8)NEXT_PLAYER_Y][(u8)NEXT_PLAYER_X];
        if ((next_pixel.red != COLOR_WALL.red) ||
            (next_pixel.green != COLOR_WALL.green) ||
            (next_pixel.blue != COLOR_WALL.blue))
        {
            PLAYER_X = NEXT_PLAYER_X;
            PLAYER_Y = NEXT_PLAYER_Y;
            return;
        }
    }
    // NOTE: Not the best solution. Would be nicer to simply prevent player
    // from being able to move diagonally.
    {
        Pixel next_pixel = buffer[(u8)PLAYER_Y][(u8)NEXT_PLAYER_X];
        if ((next_pixel.red != COLOR_WALL.red) ||
            (next_pixel.green != COLOR_WALL.green) ||
            (next_pixel.blue != COLOR_WALL.blue))
        {
            PLAYER_X = NEXT_PLAYER_X;
            NEXT_PLAYER_Y = PLAYER_Y;
            return;
        }
    }
    {
        Pixel next_pixel = buffer[(u8)NEXT_PLAYER_Y][(u8)PLAYER_X];
        if ((next_pixel.red != COLOR_WALL.red) ||
            (next_pixel.green != COLOR_WALL.green) ||
            (next_pixel.blue != COLOR_WALL.blue))
        {
            NEXT_PLAYER_X = PLAYER_X;
            PLAYER_Y = NEXT_PLAYER_Y;
            return;
        }
    }
    NEXT_PLAYER_X = PLAYER_X;
    NEXT_PLAYER_Y = PLAYER_Y;
}

static void update_frame(Frame* frame,
                         u8     input,
                         Pixel  buffer[PX_HEIGHT][PX_WIDTH]) {
    frame->delta += frame->start - frame->prev;
    while (FRAME_UPDATE_STEP < frame->delta) {
        if (input & INPUT_UP) {
            NEXT_PLAYER_Y -= KEY_SENSITIVITY;
        }
        if (input & INPUT_DOWN) {
            NEXT_PLAYER_Y += KEY_SENSITIVITY;
        }
        if (input & INPUT_LEFT) {
            NEXT_PLAYER_X -= KEY_SENSITIVITY;
        }
        if (input & INPUT_RIGHT) {
            NEXT_PLAYER_X += KEY_SENSITIVITY;
        }
        update_player_position(buffer);
        frame->delta -= FRAME_UPDATE_STEP;
        ++frame->update_count;
    }
    frame->prev = frame->start;
}

static void debug_frame(Frame* frame) {
    frame->end = SDL_GetTicks();
    if (FRAME_DEBUG_INTERVAL <= ++frame->fps_count) {
        printf("\033[2A"
               "frames  / sec.  :%6.2f\n"
               "updates / frame :%6.2f\n",
               (f32)frame->fps_count / (f32)(frame->end - frame->fps_start) *
                   MILLISECONDS,
               (f32)frame->update_count / (f32)FRAME_DEBUG_INTERVAL);
        frame->fps_start = frame->start;
        frame->fps_count = 0;
        frame->update_count = 0;
    }
}

static const u32 TEXTURE_WIDTH = PX_WIDTH * sizeof(Pixel);

static void loop(SDL_Renderer* renderer,
                 SDL_Texture*  texture,
                 Pixel         buffer[PX_HEIGHT][PX_WIDTH]) {
    Frame frame = {0};
    u8    input = 0;
    printf("\n\n");
    for (;;) {
        frame.start = SDL_GetTicks();
        set_input(&input);
        if (input & INPUT_DEAD) {
            return;
        }
        update_frame(&frame, input, buffer);
        set_buffer(buffer);
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
        debug_frame(&frame);
    }
}

i32 main(void) {
    printf("sizeof(Frame)  : %zu\n"
           "sizeof(Pixel)  : %zu\n"
           "sizeof(Input)  : %zu\n"
           "sizeof(Memory) : %zu\n\n",
           sizeof(Frame),
           sizeof(Pixel),
           sizeof(Input),
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
    loop(renderer, texture, memory->buffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(memory);
    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
