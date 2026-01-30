#ifndef VMATH_H
#define VMATH_H
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef SINGLE_PRECISION
typedef float v4sf __attribute__((vector_size(16)));
#define float_t float
#define vector v4sf
#define FLOAT_T_MAX FLT_MAX
#else
typedef double v4df __attribute__((vector_size(32)));
#define float_t double
#define vector v4df
#define FLOAT_T_MAX DBL_MAX
#endif


#include <stdlib.h>

typedef union {
    vector v;
    struct {
        float_t x;
        float_t y;
        float_t z;
        float_t w;
    };
} vector_accessor;



static inline void* aligned_malloc(int align, size_t size) {
    void* p;
    int e = posix_memalign((void**)&p, align, size);
    if (e != 0) {
        fprintf(stderr, "Memory Allocation Error\n");
        exit(1);
    }
    return p;
}

#define V3(x, y, z) ((vector){x, y, z, 0.0})
#define X(v) v[0]
#define Y(v) v[1]
#define Z(v) v[2]
#define W(v) v[3]
#define V4(x, y, z, w) ((vector){x, y, z, w})
//TODO AVC or __mm_ or something
#define IS_VZERO(v) (v[0] == 0 && v[1] == 0 && v[2] == 0 && v[3] == 0)
static inline void print_vector(vector v) {
    fprintf(stderr, "{%lf, %lf, %lf, %lf}\n", v[0], v[1], v[2], v[3]);
}

static inline vector cross(vector a, vector b) {
    //TODO AVX
    return V3(
                a[1] * b[2] + a[2] * b[1],
                a[2] * b[0] + a[0] * b[1],
                a[0] * b[1] + a[1] * b[0]
            );
}

static inline float_t dot(vector a, vector b) {
    //TODO AVX
    vector c = a * b;
    return (c[0] + c[1] + c[2] + c[3]);
}

static inline float_t length(vector a) {
    return sqrt(dot(a, a));
}

static inline vector normalize(vector a) {
    float_t l = length(a);
    return a / l;
}

static inline vector reflect(vector v, vector n) {
    float_t d = dot(v, n);
    return (v - (d * (float_t)2.0 * n));
}


typedef struct {
    float_t ax, az;
} aimY;

static inline vector rotate(vector point, vector axis, float_t theta) {
    vector p = point, a = axis;
    float_t x = X(p);
    float_t y = Y(p);
    float_t z = Z(p);
    float_t u = X(a);
    float_t v = Y(a);
    float_t w = Z(a);
    return     V3((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

static inline vector rotateX(vector point, float_t theta) {
    vector p = point;
    float_t x = X(p);
    float_t y = Y(p);
    float_t z = Z(p);
    static const float_t u = 1;
    static const float_t v = 0;
    static const float_t w = 0;
    return     V3((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

static inline vector rotateY(vector point, float_t theta) {
    vector p = point;
    float_t x = X(p);
    float_t y = Y(p);
    float_t z = Z(p);
    static const float_t u = 0;
    static const float_t v = 1;
    static const float_t w = 0;
    return     V3((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

static inline vector rotateZ(vector point, float_t theta) {
    vector p = point;
    float_t x = X(p);
    float_t y = Y(p);
    float_t z = Z(p);
    static const float_t u = 0;
    static const float_t v = 0;
    static const float_t w = 1;
    return     V3((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}


static inline vector applyAimY(aimY a, vector p) {
    p = rotateX(p, a.ax);
    p = rotateZ(p, a.az);
    return p;
}

static inline aimY computeAimY(vector normal) {
    vector n = normal;
    float_t nlength = length(normal);
    float_t xylength = length(V3(X(n), Y(n), 0.0));
    float_t ax, az;
    if (xylength == 0)
        az = X(n) > 0 ? M_PI/2 : -M_PI/2;
    else
        az = acos(Y(n))/xylength;
    ax = acos(xylength/nlength);
    ax = Z(n) > 0 ? ax : -ax;
    az = Z(n) > 0 ? -az : az;

    return (aimY){ax, az};
}


#endif
