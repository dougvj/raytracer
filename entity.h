#ifndef ENTITY_H
#define ENTITY_H
#include "vmath.h"

typedef struct entity_t entity;

typedef struct {
    vector __attribute((aligned(64))) c_reflect;
    vector __attribute((aligned(64))) c_diffuse;
    vector __attribute((aligned(64))) c_emissions;
    float intensity;
} material;


typedef struct {
    vector __attribute((aligned(64))) p;
    float r;
    material m;
    entity* e;
} sphere;

typedef struct {
    vector __attribute((aligned(64))) p;
    vector __attribute((aligned(64))) n;
    material m1;
    material m2;
    entity* e;
} plane;

typedef struct {
    vector __attribute((aligned(64))) v1;
    vector __attribute((aligned(64))) v2;
    vector __attribute((aligned(64))) v3;
    vector __attribute((aligned(64))) n;
    material m;
    entity* e;
} triangle;

#define PLANE 0x1
#define SPHERE 0x2
#define TRIANGLE 0x3


struct entity_t {
    int type;
    union {
        triangle* t;
        plane* p;
        sphere* s;
    };
};



#endif
