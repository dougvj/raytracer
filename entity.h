#ifndef ENTITY_H
#define ENTITY_H
#include "vmath.h"

typedef struct {
    float4 c_reflect;
    float4 c_diffuse;
    float4 c_emissions;
} material;


typedef struct {
    float4 p;
    float r;
    material m;
} sphere;

typedef struct {
    float4 p;
    float4 n;
    material m;
} plane;

typedef struct {
    float4 v1;
    float4 v2;
    float4 v3;
    float4 n;
    material m;
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
} entity;



#endif
