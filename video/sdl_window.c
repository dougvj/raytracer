#include "../window.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <assert.h>

struct window_t {
    int x, y;
    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    SDL_Texture* out_buffer;
};

window* initWindow(int x, int y) {
    fprintf(stderr, "Window initialized at %i x %i\n", x, y);
    window* w = malloc(sizeof(struct window_t));
 /*   if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return NULL;
    } */
    w->x = x;
    w->y = y;
    w->sdl_window = SDL_CreateWindow("raytracer", 0, 0, x, y, SDL_WINDOW_RESIZABLE);
    if (!w->sdl_window) {
        goto fail;
    }
    return w;
fail:
    fprintf(stderr, "Could not initialize SDL window: %s\n", SDL_GetError());
    SDL_Quit();
    free(w);
    return NULL;
}


int windowSetDrawingThread(window *w) {
    w->sdl_renderer = SDL_CreateRenderer(w->sdl_window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!w->sdl_renderer) {
        fprintf(stderr, "Couldn't create SDL Renderer: %s\n", SDL_GetError());
    }
    w->out_buffer = SDL_CreateTexture(w->sdl_renderer, 
                                            SDL_PIXELFORMAT_RGB24,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            w->x, w->y);
    if (!w->out_buffer)
        return 0;
    return !!(w->sdl_renderer);
}

void windowFreeDrawingThread(window* w) {
    SDL_DestroyTexture(w->out_buffer); 
    SDL_DestroyRenderer(w->sdl_renderer);
}

void windowDrawBuffer(window* w, const char* buf, int sx, int sy, rect r) {

    /*    Uint32 format;
    int tx, ty;
    SDL_QueryTexture(buffer, &format, NULL, &tx, &ty);
    assert(tx == sx);
    assert(ty == sy);
    assert(format == SDL_PIXELFORMAT_RGB24);*/
/*    int length = 0;
    Uint32* update_buf = NULL;
    GLint success = SDL_LockTexture(w->out_buffer,
                                    NULL,
                                    (void**)&update_buf,
                                    &length);
    if (success > 0)
        goto fail;
    memcpy(update_buf, buf, sx * sy * 3); 
    SDL_UnlockTexture(w->out_buffer);*/
    SDL_UpdateTexture(w->out_buffer, NULL, buf, sx * 3);
    SDL_RenderCopy(w->sdl_renderer, w->out_buffer, NULL, NULL); 
    return;
/*fail:
    fprintf(stderr, "Could not update texture: %s\n", SDL_GetError());*/
}

void windowDrawRect(window* w, rect r) {
    SDL_Rect rect;
    rect.w = r.w;
    rect.h = r.h;
    rect.x = r.x;
    rect.y = r.y;
    SDL_SetRenderDrawColor(w->sdl_renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(w->sdl_renderer, &rect);
}

void windowUpdate(window* w) {
    SDL_RenderPresent(w->sdl_renderer);
}

void freeWindow(window* w) {
    fprintf(stderr, "Window freed\n");
    SDL_Quit();
    free(w);
}

int windowCheckQuit(window* w) {
    SDL_Event e;
    int quit = 0;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = 1;
        }
    }
    return quit;
}
