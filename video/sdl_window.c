#include <SDL/SDL.h>
#include "../window.h"

struct window_t window {
    SDL_Window* window;
    SDL_GLContext* gl;
};


window* initWindow(int x, int y);
void drawBuffer(window* w, const char* buf, int sx, int sy, rect view_port);
void drawRect(window* w, rect r);

