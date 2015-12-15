#ifndef VMATH_H
#define VMATH_H
#include "math.h"
#include <stdio.h>
typedef double v4sd __attribute__((vector_size(32)));

typedef union {
    v4sd v;
    double e[4];
    struct {
        double x;
        double y;
        double z;
        double w;
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

inline double dot(float4 a, float4 b) {
    float4 c = {.v = a.v * b.v};
    return (c.e[0] + c.e[1] + c.e[2] + c.e[3]);
}

inline double length(float4 a) {
    return sqrt(dot(a, a));
}

inline float4 normalize(float4 a) {
    double l = length(a);
    return (float4){.v = a.v / FLOAT4_f(l).v};
}

inline float4 reflect(float4 v, float4 n) {
    double d = dot(v, n);
    return (float4){.v = (v.v - (FLOAT4_f(d).v * n.v * FLOAT4_f(2.0f).v))};
}

inline int isZero(float4 f) {
    return (f.x == 0.0f && f.y == 0.0f && f.z == 0.0f && f.w == 0.0f);
}

typedef struct {
    double ax, az;
} aimY;

inline float4 rotate(float4 point, float4 axis, double theta) {
    double x = point.x;
    double y = point.y;
    double z = point.z;
    double u = axis.x;
    double v = axis.y;
    double w = axis.z;
    return FLOAT4_3f((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline float4 rotateX(float4 point, double theta) {
    double x = point.x;
    double y = point.y;
    double z = point.z;
    const double u = 1;
    const double v = 0;
    const double w = 0;
    return FLOAT4_3f((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline float4 rotateZ(float4 point, double theta) {
    double x = point.x;
    double y = point.y;
    double z = point.z;
    const double u = 0;
    const double v = 0;
    const double w = 1;
    return FLOAT4_3f((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline float4 applyAimY(aimY a, float4 p) {
    p = rotateX(p, a.ax);
    p = rotateZ(p, a.az);
    return p;
}

inline aimY computeAimY(float4 n) {
    double nlength = length(n);
    double xylength = length(FLOAT4_3f(n.x, n.y, 0.0f));
    double ax, az;
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
