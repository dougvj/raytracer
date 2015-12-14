#ifndef VMATH_H
#define VMATH_H
#include "math.h"

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
    return (float4){.v = a.v / l};
}

inline float4 reflect(float4 v, float4 n) {
    return (float4){.v = (v.v - (dot(v, n) * n.v * 2.0f))};
}

#endif
