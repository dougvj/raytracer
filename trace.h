#ifndef TRACE_H
#define TRACE_H
#include "vmath.h"


typedef struct {
    float4 c_reflect;
    float4 c_diffuse;
    float4 c_emissions;
} material;

typedef struct {
    float4 p;
    float r;
} sphere;

typedef struct {
    float4 p;
    float4 n;
} plane;

typedef struct {
    float4 v1;
    float4 v2;
    float4 v3;
    float4 n;
} triangle;

#define PLANE 0x1
#define SPHERE 0x2
#define TRIANGLE 0x3


typedef struct {
    int type;
    union {
        triangle* t;
        plane* p;
        sphere* s;
    };
    material* m;
} entity;

typedef struct {
    float4 p;
    float4 d;
} ray;

typedef struct {
     int num_threads;
     int x;
     int y;
     color** output;
} render_context;

#endif
