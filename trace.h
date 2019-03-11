#ifndef TRACE_H
#define TRACE_H
#include "vmath.h"
#include "geometric_primitives.h"
#include "window.h"

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
void deleteRenderContext(render_context* rc);

typedef struct {
    int x;
    int y;
    int num_threads;
    int max_bounces;
    int samples_per_pixel;
    char* output_buffer;
    float origin_x;
    float origin_y;
    float origin_z;
    plane* planes;
    int num_planes;
    triangle* triangles;
    int num_triangles;
    sphere* spheres;
    int num_spheres;
    window* w;
} render_parameters;

int renderScene(render_context* rc, render_parameters* params);

#endif
