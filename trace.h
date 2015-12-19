#ifndef TRACE_H
#define TRACE_H
#include "vmath.h"
#include "geometric_primitives.h"
#include "libdatastruct/linkedlist.h"


typedef struct __attribute__((aligned(64))) {
    vector p;
    vector d;
} ray;

typedef union {
    unsigned char c[3];
    struct {
        unsigned char b, g, r;
    };
} color;

typedef struct render_context_t render_context;


render_context* createRenderContext();
void addSphere(render_context* rc, sphere* s);
void addPlane(render_context* rc, plane* p);
void addTriangle(render_context* rc, triangle* t);
void renderScene(render_context* c, int x, int y, int num_threads, int max_iterations);

#endif
