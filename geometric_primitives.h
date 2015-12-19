#ifndef ENTITY_H
#define ENTITY_H
#include "vmath.h"


typedef struct __attribute((aligned(64)))  {
    vector c_reflect;
    vector c_diffuse;
    vector c_emissions;
} material;


typedef struct __attribute__((aligned(64))) {
    vector p;
    float r;
    material m;
} sphere;

typedef struct __attribute__((aligned(64))) {
    vector p;
    vector n;
    material m1;
    material m2;
} plane;

typedef struct __attribute__((aligned(64))) {
    vector v1;
    vector v2;
    vector v3;
    vector n;
    material m;
} triangle;


sphere* createSphere(vector pos, float_t r, material m);
plane* createPlane(vector pos, vector norm, material m1, material m2);

#endif

