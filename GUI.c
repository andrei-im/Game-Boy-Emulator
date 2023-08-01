#include "GUI.h"
#include "Emulator.h"
#include "MemoryBus.h"
#include "GPU.h"
#include "Input.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include "LCD.h"


SDL_Window* sdlWindow;
SDL_Window* sdlDebugWindow;

SDL_Renderer* sdlRenderer;
SDL_Renderer* sdlDebugRenderer;

SDL_Texture* sdlTexture;
SDL_Texture* sdlDebugTexture;

SDL_Surface* screen;
SDL_Surface* debugScreen;

static int scale = 4;

static unsigned long colors_default[4] = {
        0xFF9bbc0f, // lightest shade
        0xFF8bac0f, // light shade
        0xFF306230, // dark shade
        0xFF0F380F  // darkest shade
};

static unsigned long colors_black[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

static unsigned long colors_green[4] = {
        0xFF98FB98, // PaleGreen
        0xFF008000, // Green
        0xFF006400, // DarkGreen
        0xFF2E8B57  // SeaGreen
};

static unsigned long colors_orange[4] = {
        0xFFFFA500, // Orange
        0xFFFF8C00, // DarkOrange
        0xFFFF4500, // OrangeRed
        0xFFFF6347  // Tomato
};

static unsigned long colors_purple[4] = {
        0xFFE6E6FA, // Lavender
        0xFF800080, // Purple
        0xFF4B0082, // Indigo
        0xFF483D8B  // DarkSlateBlue
};

void updateWindowTitle(uint32_t fps) {
    char title[50];
    sprintf(title, "Gameboy Emulator - FPS: %d", fps);
    SDL_SetWindowTitle(sdlWindow, title);
}

void initGUI() {

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale, SDL_WINDOW_RESIZABLE, &sdlWindow, &sdlRenderer);

    SDL_SetWindowTitle(sdlWindow, "Gameboy emulator");

    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH * scale,
                                  SCREEN_HEIGHT * scale,
                                  32,
                                  0x00FF0000,
                                  0x0000FF00,
                                  0x000000FF,
                                  0xFF000000);
    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   SCREEN_WIDTH * scale,
                                   SCREEN_HEIGHT * scale);

    SDL_CreateWindowAndRenderer(16 * 8 * scale,
                                32 * 8 * scale,
                                SDL_WINDOW_HIDDEN,
                                &sdlDebugWindow,
                                &sdlDebugRenderer);

    SDL_SetWindowTitle(sdlDebugWindow, "GPU Debug");

    debugScreen = SDL_CreateRGBSurface(0,
                                       (16 * 8 * scale) + (16 * scale),
                                       (32 * 8 * scale) + (64 * scale),
                                       32,
                                       0x00FF0000,
                                       0x0000FF00,
                                       0x000000FF,
                                       0xFF000000);

    sdlDebugTexture = SDL_CreateTexture(sdlDebugRenderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        (16 * 8 * scale) + (16 * scale),
                                        (32 * 8 * scale) + (64 * scale));

    int x, y;
    SDL_GetWindowPosition(sdlWindow, &x, &y);
    SDL_SetWindowPosition(sdlDebugWindow, x + SCREEN_WIDTH * scale + 10, y);
}

void delay(uint32_t ms) {
    SDL_Delay(ms);
}

uint32_t getTicks() {
    return SDL_GetTicks();
}

static unsigned long tileColors[4] = {
        0xFF9bbc0f, // lightest shade
        0xFF8bac0f, // light shade
        0xFF306230, // dark shade
        0xFF0F380F  // darkest shade
};

void displayTile(SDL_Surface *surface, uint16_t startLocation, uint16_t tileNum, int x, int y) {
    SDL_Rect rc;

    for (int tileY=0; tileY<16; tileY += 2) {
        uint8_t b1 = ReadByteMemBus(startLocation + (tileNum * 16) + tileY);
        uint8_t b2 = ReadByteMemBus(startLocation + (tileNum * 16) + tileY + 1);

        for (int bit=7; bit >= 0; bit--) {
            uint8_t hi = !!(b1 & (1 << bit)) << 1;
            uint8_t lo = !!(b2 & (1 << bit));

            uint8_t color = hi | lo;

            rc.x = x + ((7 - bit) * scale);
            rc.y = y + (tileY / 2 * scale);
            rc.w = scale;
            rc.h = scale;

            SDL_FillRect(surface, &rc, tileColors[color]);
        }
    }
}

void updateGPUDebugW() {
    int xDraw = 0;
    int yDraw = 0;
    int tileNum = 0;

    SDL_Rect rc;
    rc.x = 0;
    rc.y = 0;
    rc.w = debugScreen->w;
    rc.h = debugScreen->h;
    SDL_FillRect(debugScreen, &rc, 0xFF111111);

    uint16_t addr = 0x8000;

    //384 tiles, 24 x 16
    for (int y=0; y<24; y++) {
        for (int x=0; x<16; x++) {
            displayTile(debugScreen, addr, tileNum, xDraw + (x * scale), yDraw + (y * scale));
            xDraw += (8 * scale);
            tileNum++;
        }

        yDraw += (8 * scale);
        xDraw = 0;
    }

    SDL_UpdateTexture(sdlDebugTexture, NULL, debugScreen->pixels, debugScreen->pitch);
    SDL_RenderClear(sdlDebugRenderer);
    SDL_RenderCopy(sdlDebugRenderer, sdlDebugTexture, NULL, NULL);
    SDL_RenderPresent(sdlDebugRenderer);
}

void updateEmuWindow() {
    SDL_Rect rect;
    rect.x = rect.y = 0;      // TODO remove
    rect.w = rect.h = 0;

    uint32_t* videoBuffer = getGPUState()->videoBuffer;

    for (int scanlineNum = 0; scanlineNum < YRES; scanlineNum++) {
        for (int x = 0; x < XRES; x++) {
            rect.x = x * scale;
            rect.y = scanlineNum * scale;
            rect.w = scale;
            rect.h = scale;

            SDL_FillRect(screen, &rect, videoBuffer[x + (scanlineNum * XRES)]);
        }
    }

    SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);

    updateGPUDebugW();
}

void handleGameInputs(bool down, uint32_t key_code) {

    switch (key_code) {
        case SDLK_z:
            getGamepadButtonState()->b = down;
            break;
        case SDLK_x:
            getGamepadButtonState()->a = down;
            break;
        case SDLK_RETURN:
            getGamepadButtonState()->start = down;
            break;
        case SDLK_SPACE:
            getGamepadButtonState()->select = down;
            break;
        case SDLK_UP:
            getGamepadButtonState()->up = down;
            break;
        case SDLK_DOWN:
            getGamepadButtonState()->down = down;
            break;
        case SDLK_LEFT:
            getGamepadButtonState()->left = down;
            break;
        case SDLK_RIGHT:
            getGamepadButtonState()->right = down;
            break;
    }
}

void handleExtraKeyInputs(uint32_t key_code) {

    switch (key_code) {
        case SDLK_d: {
            Uint32 windowFlags = SDL_GetWindowFlags(sdlDebugWindow);
            if (windowFlags & SDL_WINDOW_SHOWN) {
                SDL_HideWindow(sdlDebugWindow);
            } else {
                SDL_ShowWindow(sdlDebugWindow);
            }
            break;
        }

        case SDLK_q: {
            getEmulatorState()->die = true;
            break;
        }

        case SDLK_1: {
            for (int i=0; i<4; i++) {
                getLCDState()->bgColors[i] = colors_default[i];
                getLCDState()->sp1Colors[i] = colors_default[i];
                getLCDState()->sp2Colors[i] = colors_default[i];
            }
            break;
        }
        case SDLK_2: {
            for (int i=0; i<4; i++) {
                getLCDState()->bgColors[i] = colors_black[i];
                getLCDState()->sp1Colors[i] = colors_black[i];
                getLCDState()->sp2Colors[i] = colors_black[i];
            }
            break;
        }
        case SDLK_3: {
            for (int i=0; i<4; i++) {
                getLCDState()->bgColors[i] = colors_green[i];
                getLCDState()->sp1Colors[i] = colors_green[i];
                getLCDState()->sp2Colors[i] = colors_green[i];
            }
            break;
        }
        case SDLK_4: {
            for (int i=0; i<4; i++) {
                getLCDState()->bgColors[i] = colors_orange[i];
                getLCDState()->sp1Colors[i] = colors_orange[i];
                getLCDState()->sp2Colors[i] = colors_orange[i];
            }
            break;
        }
        case SDLK_5: {
            for (int i=0; i<4; i++) {
                getLCDState()->bgColors[i] = colors_purple[i];
                getLCDState()->sp1Colors[i] = colors_purple[i];
                getLCDState()->sp2Colors[i] = colors_purple[i];
            }
            break;
        }

        case SDLK_p: {
            if (getEmulatorState()->paused == false) {
                getEmulatorState()->paused = true;
            } else {
                getEmulatorState()->paused = false;
            }
            break;
        }
    }
}

void handleEventsGUI() {
    SDL_Event e;
    while (SDL_PollEvent(&e) > 0)
    {
        if (e.type == SDL_KEYDOWN) {
            handleGameInputs(true, e.key.keysym.sym);
            handleExtraKeyInputs(e.key.keysym.sym);

        }

        if (e.type == SDL_KEYUP) {
            handleGameInputs(false, e.key.keysym.sym);
        }

        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            getEmulatorState()->die = true;
        }
    }
}