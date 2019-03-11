#include "../window.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>

struct window_t {
    SDL_Window* sdl_window;
    SDL_GLContext gl_context;
};

window* initWindow(int x, int y) {
    fprintf(stderr, "Window initialized at %i x %i\n", x, y);
    window* w = malloc(sizeof(struct window_t));
 /*   if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return NULL;
    } */
    w->sdl_window = SDL_CreateWindow("raytracer", 0, 0, x, y, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    w->gl_context = SDL_GL_CreateContext(w->sdl_window);
    if (!w->gl_context) {
        fprintf(stderr, "Couldn't create OpenGL context: %s\n", SDL_GetError());
    }
    return !!(w->gl_context);
}

void windowFreeDrawingThread(window* w) {
    SDL_GL_DeleteContext(w->gl_context);
}

void windowDrawBuffer(window* w, const char* buf, int sx, int sy, rect r) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_MakeCurrent(w->sdl_window, w->gl_context);
    fprintf(stderr, "Told to draw a buffer of size %i, %i at location %i %i and size %i %i\n", sx, sy, r.x, r.y, r.w, r.h );
}

void windowDrawRect(window* w, rect r) {
    float verticies[] = {
        (float)r.x, (float)r.y
    }
    glDrawArrays(GL_LINES, 0, 4);

    fprintf(stderr, "Told to draw a rect at location %i %i and size %i %i\n", r.x, r.y, r.w, r.h );
}

void windowUpdate(window* w) {
    SDL_GL_SwapWindow(w->sdl_window);
    fprintf(stderr, "Told to update window\n");
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
