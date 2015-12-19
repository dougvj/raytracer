#include "geometric_primitives.h"

sphere* createSphere(vector pos, float_t r, material m) {
    sphere* s = aligned_malloc(64, sizeof(sphere));
    s->p = pos;
    s->r = r;
    s->m = m;
    return s;
}

plane* createPlane(vector pos, vector norm, material m1, material m2) {
    plane* p = aligned_malloc(64, sizeof(plane));
    p->p = pos;
    p->n = norm;
    p->m1 = m1;
    p->m2 = m2;
    return p;
}
