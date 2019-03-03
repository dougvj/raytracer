#ifndef TRACE_H
#define TRACE_H
#include "vmath.h"
#include "geometric_primitives.h"
#include "libdatastruct/linkedlist.h"


typedef struct __attribute__((aligned(64))) {
    vector p; //position
    vector d; //direction
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


typedef struct {
    int x;
    int y;
    int num_threads;
    int max_bounces;
    int rays_per_pixel;
    char* output_buffer;
    float origin_x;
    float origin_y;
    float origin_z;
    int frame;
} render_parameters;
void renderScene(render_context* rc, render_parameters* params);

#endif
