#ifndef VMATH_H
#define VMATH_H
#include "math.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef SINGLE_PRECISION
typedef float vector __attribute__((vector_size(16)));
#define float_t float
#else
typedef double vector __attribute__((vector_size(32)));
#define float_t double
#endif


#include <stdlib.h>

typedef union {
    vector v;
    float_t e[4];
    struct {
        float_t x;
        float_t y;
        float_t z;
        float_t w;
    };
} vector_accessor;



inline void* aligned_malloc(int align, size_t size) {
    void* p;
    int e = posix_memalign((void**)&p, align, size);
    if (e != 0) {
        fprintf(stderr, "Memory Allocation Error\n");
        exit(1);
    }
    return p;
}

#define VEC3F(x, y, z) ((vector){x, y, z, 0.0})
#define COMPONENT(x) ((vector_accessor){.v = x})
#define VEC4F(x, y, z, w) ((vector){x, y, z, w})
#define SCALAR(x) (vector){x, x, x, x}
#define ZERO_VECTOR() (vector){0.0, 0.0, 0.0, 0.0}

inline void print_vector(vector vect) {
    vector_accessor v = COMPONENT(vect);
    fprintf(stderr, "{%lf, %lf, %lf, %lf}\n", v.x, v.y, v.z, v.w);
}

inline vector cross(vector av, vector bv) {
    vector_accessor a = COMPONENT(av);
    vector_accessor b = COMPONENT(bv);

    return VEC3F(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline float_t dot(vector a, vector b) {
    vector_accessor c = COMPONENT(a * b);
    return (c.e[0] + c.e[1] + c.e[2] + c.e[3]);
}

inline float_t length(vector a) {
    return sqrt(dot(a, a));
}

inline vector normalize(vector a) {
    float_t l = length(a);
    return a / SCALAR(l);
}

inline vector reflect(vector v, vector n) {
    float_t d = dot(v, n);
    return (v - (SCALAR(d) * n * SCALAR(2.0)));
}

inline int isZero(vector v) {
    vector_accessor f = COMPONENT(v);
    return (f.x == 0.0f && f.y == 0.0f && f.z == 0.0f && f.w == 0.0f);
}

typedef struct {
    float_t ax, az;
} aimY;

inline vector rotate(vector point, vector axis, float_t theta) {
    vector_accessor p = COMPONENT(point);
    vector_accessor a = COMPONENT(axis);
    float_t x = p.x;
    float_t y = p.y;
    float_t z = p.z;
    float_t u = a.x;
    float_t v = a.y;
    float_t w = a.z;
    return     VEC3F((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline vector rotateX(vector point, float_t theta) {
    vector_accessor p = COMPONENT(point);
    float_t x = p.x;
    float_t y = p.y;
    float_t z = p.z;
    static const float_t u = 1;
    static const float_t v = 0;
    static const float_t w = 0;
    return     VEC3F((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline vector rotateY(vector point, float_t theta) {
    vector_accessor p = COMPONENT(point);
    float_t x = p.x;
    float_t y = p.y;
    float_t z = p.z;
    static const float_t u = 0;
    static const float_t v = 1;
    static const float_t w = 0;
    return     VEC3F((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline vector rotateZ(vector point, float_t theta) {
    vector_accessor p = COMPONENT(point);
    float_t x = p.x;
    float_t y = p.y;
    float_t z = p.z;
    static const float_t u = 0;
    static const float_t v = 0;
    static const float_t w = 1;
    return     VEC3F((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}


inline vector applyAimY(aimY a, vector p) {
    p = rotateX(p, a.ax);
    p = rotateZ(p, a.az);
    return p;
}

inline aimY computeAimY(vector normal) {
    vector_accessor n = COMPONENT(normal);
    float_t nlength = length(normal);
    float_t xylength = length(VEC3F(n.x, n.y, 0.0));
    float_t ax, az;
    if (xylength == 0)
        az = n.x > 0 ? M_PI/2 : -M_PI/2;
    else
        az = acos(n.y/xylength);
    ax = acos(xylength/nlength);
    ax = n.z > 0 ? ax : -ax;
    az = n.z > 0 ? -az : az;

    return (aimY){ax, az};
}


#endif
