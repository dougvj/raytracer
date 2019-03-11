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
int windowSetDrawingThread(window* w);
void windowFreeDrawingThread(window* w);
void windowDrawBuffer(window* w, const char* buf, int sx, int sy, rect r);
void windowDrawRect(window* w, rect r);
void windowUpdate(window* w);
void freeWindow(window* w);
int windowCheckQuit(window* w);

#endif
