#ifndef VMATH_H
#define VMATH_H
#include "math.h"
#include <stdio.h>
typedef float v4sf __attribute__((vector_size(16)));

typedef union {
    v4sf v;
    float e[4];
    struct {
        float x;
        float y;
        float z;
        float w;
    };
} float4;

#define FLOAT4_3f(x, y, z) (float4){.v = {x, y, z, 0.0f}}
#define FLOAT4_f(x) (float4){.v = {x, x, x, x}}
#define FLOAT4_4f(x, y, z, w) (float4){.v = {x, y, z, w}}
#define FLOAT4_v(x) (float4){.v = x}
#define FLOAT4_Zero() (float4){.v = {0.0f, 0.0f, 0.0f, 0.0f}}

inline void print_float4(float4 f) {
    fprintf(stderr, "{%f, %f, %f, %f}\n", f.x, f.y, f.z, f.w);
}

inline float4 cross(float4 a, float4 b) {
    return (float4){.e = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}};
}

inline float dot(float4 a, float4 b) {
    float4 c = {.v = a.v * b.v};
    return (c.e[0] + c.e[1] + c.e[2] + c.e[3]);
}

inline float length(float4 a) {
    return sqrt(dot(a, a));
}

inline float4 normalize(float4 a) {
    float l = length(a);
    return (float4){.v = a.v / FLOAT4_f(l).v};
}

inline float4 reflect(float4 v, float4 n) {
    float d = dot(v, n);
    return (float4){.v = (v.v - (FLOAT4_f(d).v * n.v * FLOAT4_f(2.0f).v))};
}

#endif
