#include "trace.h"
#include <stdlib.h>

float4 intersectSphere(sphere* s, ray* r) {
    float a = dot(r->d, r->d);
    float4 rs = (float4){.v = r->p.v - s->p.v};
    float b = dot((float4){.v = 2.0f * r->d.v}, rs);
    float c = dot(rs, rs) - s->r*s->r;
    float disc = (b*b) - (4 * a * c);
    if (disc >= 0) {
        float t = ((-b) - sqrt(disc))/(2*a);
        return (float4){.v = {r->p.x + t * r->d.x, r->p.y + t * r->d.y, r->p.z + t * r->d.z, 0.0f}};
    }
    return (float4){.v = {0, 0, 0, 0}};
}

float4 intersectPlane(plane* p, ray* r) {
    float d = dot(p->n, (float4){.v = r->d.v - r->p.v});
    if (d == 0)
        return (float4){.v = {0, 0, 0, 0}};
    float n = dot(p->n, (float4){.v = p->p.v - r->p.v});
    float r1 = n/d;
    if (r1 < 0)
        return (float4){.v = {0, 0, 0, 0}};
    return (float4){.v = r->p.v + r1 * (r->d.v - r->p.v)};
}

float4 intersectTriangle(triangle* t, ray* r) {
    return (float4){.v = {0, 0, 0, 0}};

}

float4 intersect(entity* e, ray* r) {
    switch(e->type) {
        case TRIANGLE:
            return intersectTriangle(e->t, r);
            break;
        case SPHERE:
            return intersectSphere(e->s, r);
            break;
        case PLANE:
            return intersectPlane(e->p, r);
            break;
    }
    return (float4){.v = {0, 0, 0, 0}};
}

typedef union {
    char c[3];
    struct {
        char r, g, b;
    };
} color;



void renderScene(int x, int y, int num_threads) {
    color** output;
    output = malloc(sizeof(color) * x);
    for (int i = 0; i < y; i++) {
         output[i] = malloc(sizeof(color) * y);
    }

}
