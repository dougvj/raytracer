#ifndef TRACE_H
#define TRACE_H
#include "vmath.h"
#include "entity.h"
#include "libdatastruct/linkedlist.h"


typedef struct {
    float4 p;
    float4 d;
} ray;

typedef union {
    unsigned char c[3];
    struct {
        unsigned char b, g, r;
    };
} color;

typedef struct {
     int num_threads;
     int x;
     int y;
     double fov;
     LL* entities;
     color* output;
} render_context;

render_context* createRenderContext();
void addEntity(render_context* rc, entity* e);
entity* createSphere(float4 pos, double r, material m);
entity* createPlane(float4 pos, float4 norm, material m1, material m2);
void renderScene(render_context* c, int x, int y, int num_threads);

#endif
