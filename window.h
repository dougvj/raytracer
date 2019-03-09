#ifndef _WINDOW_H_
#define _WINDOW_H_

typedef struct window_t window;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect;

window* initWindow(int x, int y);
void drawBuffer(window* w, const char* buf, int sx, int sy, rect view_port);
void drawRect(window* w, rect r);

#endif
