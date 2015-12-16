#ifndef VMATH_H
#define VMATH_H
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
typedef double vector __attribute__((vector_size(32)));
#include <stdlib.h>

typedef union {
    vector v;
    double e[4];
    struct {
        double x;
        double y;
        double z;
        double w;
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

inline double dot(vector a, vector b) {
    vector_accessor c = COMPONENT(a * b);
    return (c.e[0] + c.e[1] + c.e[2] + c.e[3]);
}

inline double length(vector a) {
    return sqrt(dot(a, a));
}

inline vector normalize(vector a) {
    double l = length(a);
    return a / SCALAR(l);
}

inline vector reflect(vector v, vector n) {
    double d = dot(v, n);
    return (v - (SCALAR(d) * n * SCALAR(2.0)));
}

inline int isZero(vector v) {
    vector_accessor f = COMPONENT(v);
    return (f.x == 0.0f && f.y == 0.0f && f.z == 0.0f && f.w == 0.0f);
}

typedef struct {
    double ax, az;
} aimY;

inline vector rotate(vector point, vector axis, double theta) {
    vector_accessor p = COMPONENT(point);
    vector_accessor a = COMPONENT(axis);
    double x = p.x;
    double y = p.y;
    double z = p.z;
    double u = a.x;
    double v = a.y;
    double w = a.z;
    return     VEC3F((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline vector rotateX(vector point, double theta) {
    vector_accessor p = COMPONENT(point);
    double x = p.x;
    double y = p.y;
    double z = p.z;
    static const double u = 1;
    static const double v = 0;
    static const double w = 0;
    return     VEC3F((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline vector rotateY(vector point, double theta) {
    vector_accessor p = COMPONENT(point);
    double x = p.x;
    double y = p.y;
    double z = p.z;
    static const double u = 0;
    static const double v = 1;
    static const double w = 0;
    return     VEC3F((-u * (-u*x - v*y - w*z)) * (1 - cos(theta)) + x*cos(theta) + (-w*y + v*z)* sin(theta),
                     (-v * (-u*x - v*y - w*z)) * (1 - cos(theta)) + y*cos(theta) + (w*x - u*z) * sin(theta),
                     (-w * (-u*x - v*y - w*z)) * (1 - cos(theta)) + z*cos(theta) + (-v*x + u*y) * sin(theta));

}

inline vector rotateZ(vector point, double theta) {
    vector_accessor p = COMPONENT(point);
    double x = p.x;
    double y = p.y;
    double z = p.z;
    static const double u = 0;
    static const double v = 0;
    static const double w = 1;
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
    double nlength = length(normal);
    double xylength = length(VEC3F(n.x, n.y, 0.0));
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
