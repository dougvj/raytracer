#ifndef ENTITY_H
#define ENTITY_H
#include "vmath.h"

typedef struct entity_t entity;

typedef struct {
    vector c_reflect;
    vector c_diffuse;
    vector c_emissions;
    float intensity;
} material;


typedef struct {
    vector p;
    float r;
    material m;
    entity* e;
} sphere;

typedef struct {
    vector p;
    vector n;
    material m1;
    material m2;
    entity* e;
} plane;

typedef struct {
    vector v1;
    vector v2;
    vector v3;
    vector n;
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
