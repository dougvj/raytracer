#include "trace.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "bmp.h"
#include <string.h>

#define MAX_ITR 3
#define DIFFUSE_RES_PER_DEG_ARC 0.5f;
#define FOV_RADS 1.5708
typedef struct {
    int hit;
    vector n;
    vector p;
    material* m;
    entity* e;
} intersection;

intersection noHit() {
    return (intersection){0, ZERO_VECTOR(), ZERO_VECTOR(), NULL, NULL};
}

intersection Hit(vector pos, vector norm, material* m, entity* e) {
    return (intersection){1, norm, pos, m, e};
}

intersection intersectSphere(sphere* s, ray r) {
    vector rs = s->p - r.p;
    if (dot(r.d, s->p - r.p) <= 0)
       return noHit();
    double a = dot(r.d, r.d);
    rs = -rs;
    double b = dot(SCALAR(2.0) * r.d, rs);
    double c = dot(rs, rs) - s->r*s->r;
    double disc = (b*b) - (4 * a * c);
    if (disc >= 0) {
        double t = ((-b) - sqrt(disc))/(2*a);
        vector p = VEC3F(COMPONENT(r.p).x + t * COMPONENT(r.d).x, COMPONENT(r.p).y + t * COMPONENT(r.d).y, COMPONENT(r.p).z + t * COMPONENT(r.d).z);
        vector n = p - s->p;
        return Hit(p, n, &s->m, s->e);
    }
    return noHit();
}

intersection intersectPlane(plane* p, ray r) {
    double n = dot(p->n, (p->p -  r.p));
    double d = dot(p->n, r.d);
    if (d == 0)
        return noHit();
    double r1 = n/d;
    if (r1 <= 1)
        return noHit();
    vector pos =  (r.p + SCALAR(r1) * r.d);
    material* m;
    if (abs((int)(floor(COMPONENT(pos).x))) % 2 ==  abs((int)(floor(COMPONENT(pos).z))) % 2 )
        m = &p->m1;
    else
        m = &p->m2;
    return Hit(pos, p->n, m, p->e);
}

intersection intersectTriangle(triangle* t, ray r) {
    return noHit();

}

intersection intersectEntity(entity* e, ray r) {
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
    return noHit();
}

vector _traceRay(render_context* c, ray r, int num, entity* hit);


vector findDiffuse(render_context* c, vector p, vector n, int num, entity* ignore) {
    if (num > MAX_ITR)
        return ZERO_VECTOR();
    LL_itr* itr = llInitIterator(c->entities);
    entity* e = (entity*)llGetNext(itr);
    vector cor_total = ZERO_VECTOR();
    vector cor = ZERO_VECTOR();
    double distance, square;
    while (e) {
        if (e == ignore)
            goto next;
        vector light;
        ray r;
        double incidence;
        switch(e->type) {
            case SPHERE:
                light = (e->s->p - p);
                r = (ray){p, light};
                incidence = dot(normalize(light), normalize(n));
                if (incidence < 0)
                    incidence *= -1;
//                    return ZERO_VECTOR();
                cor = _traceRay(c, r, num + 2, e);
                //Apply intensity
                vector_accessor cor_a = COMPONENT(cor);
                cor_a.x *= cor_a.w;
                cor_a.y *= cor_a.w;
                cor_a.z *= cor_a.w;
                cor = cor_a.v;
                distance = length(light);
                square = 1 / (distance * distance);
                cor = (cor * SCALAR(square));
                cor_total = (cor * SCALAR(incidence)  + cor_total);
        }
next:
        e = (entity*)llGetNext(itr);
    }
    return cor_total;
}

vector getBackground(ray r) {
    return VEC4F(0.0f, 0.0f, 0.0f, 0.0f);
}

vector _traceRay(render_context* c, ray r, int num, entity* hit) {
    if (num > MAX_ITR)
        return getBackground(r);
    LL_itr* itr = llInitIterator(c->entities);
    entity* e = (entity*)llGetNext(itr);
    intersection closest = noHit();
    double distance;
    while(e) {
        intersection i = intersectEntity(e, r);
        if (i.hit) {
            double new_distance = length((i.p - r.p));
            if(closest.hit) {
                if (new_distance < distance) {
                    closest = i;
                    distance = new_distance;
                }
            }
            else {
                closest = i;
                distance = new_distance;
            }
        }
        e = (entity*)llGetNext(itr);
    }
    if (closest.hit) {
        if (e && closest.e != e)
            return ZERO_VECTOR();
        vector reflect_color;
        if (!isZero(closest.m->c_reflect))
            reflect_color = _traceRay(c, (ray){closest.p, reflect(r.d, closest.n)}, num + 1, e);
        else
            reflect_color = ZERO_VECTOR();
        vector diffuse_color;
        if (!isZero(closest.m->c_diffuse))
            diffuse_color = findDiffuse(c, closest.p, closest.n, num, closest.e);
        else
            diffuse_color = ZERO_VECTOR();
        return (closest.m->c_reflect * reflect_color +
               closest.m->c_diffuse * diffuse_color +
               closest.m->c_emissions + VEC4F(0.0f, 0.0f, 0.0f, closest.m->intensity));
    }
    return getBackground(r);
}

color convertFloatToColor(vector cor) {
    vector_accessor c = COMPONENT(cor);
    color co;
    if (c.x > 1.0f)
        c.x = 1.0f;
    if (c.y > 1.0f)
        c.y = 1.0f;
    if (c.z > 1.0f)
        c.z = 1.0f;
    co.r = c.x * 255;
    co.g = c.y * 255;
    co.b = c.z * 255;
    return co;
}

color _renderPixel(render_context* c, int x, int y) {
    double rx = (x / (double)c->x) * 2 - 1.0f;
    double ry = (((y / (double)c->y) * 2 - 1.0f) * (c->y / (float)c->x)) * -1;
    double rz = 1.0f / tan(c->fov / 2.0f);
    vector d = VEC3F(rx, ry, rz);
    ray r = {VEC3F(0.0f, 0.0f, 0.0f), d};
    return convertFloatToColor(_traceRay(c, r, 0, NULL));
}


typedef struct {
    render_context* c;
    int thread_num;
    int count_complete;
    int num_traces;
} thread_context;

void _startRenderThread(thread_context* c) {
    long total = c->c->x * c->c->y;
    //Determine block size
    long block_size_x = total / c->c->num_threads;
    long start = block_size_x * c->thread_num;
    long end = block_size_x * (c->thread_num + 1);
    for (long pixel = start; pixel < end; pixel++) {
         int x = pixel % c->c->x;
         int y = pixel / c->c->x;
         //fprintf(stderr, "%u, %u\n", x, y);
         c->c->output[pixel] = _renderPixel(c->c, x, y);
         c->count_complete++;
    }
}

render_context* createRenderContext() {
    render_context* rc = aligned_malloc(64, sizeof(render_context));
    rc->entities = llCreate();
    rc->fov = FOV_RADS;
    return rc;
}


entity* createSphere(vector pos, double r, material m) {
    sphere* s = aligned_malloc(64, sizeof(sphere));
    s->p = pos;
    s->r = r;
    entity* e = aligned_malloc(64, sizeof(entity));
    e->s = s;
    e->type = SPHERE;
    e->s->m = m;
    e->s->e = e;
    return e;
}

entity* createPlane(vector pos, vector norm, material m1, material m2) {
    plane* p = aligned_malloc(64, sizeof(plane));
    p->p = pos;
    p->n = norm;
    entity* e = aligned_malloc(64, sizeof(entity));
    e->p = p;
    e->type = PLANE;
    e->p->m1 = m1;
    e->p->m2 = m2;
    e->p->e = e;
    return e;
}

void addEntity(render_context* rc, entity* e) {
    llPushBack(rc->entities, e);
}

void renderScene(render_context* rc, int x, int y, int num_threads) {
    rc->x = x;
    rc->y = y;
    rc->num_threads = num_threads;
    color* output;
    output = aligned_malloc(64, sizeof(color) * (x * y));
    memset(output, 0, sizeof(color) * (x * y));
    rc->output = output;
    pthread_t threads[num_threads];
    thread_context contexts[num_threads];
    for (int i = 0; i < num_threads; i++) {
        thread_context* c = &contexts[i];
        c->c = rc;
        c->thread_num = i;
        c->count_complete = 0;
        c->num_traces = 0;
        pthread_create(&(threads[i]), NULL, (void *(*)(void*))_startRenderThread, (void*)c);
    }
    long complete = 0;
    long total = x * y;
    while (complete < total) {
        complete = 0;
        for (int i = 0; i < num_threads; i++) {
            complete += contexts[i].count_complete;
        }
        double percentage = complete/(double)total;
        fprintf(stderr, "%4.2f%% Complete\n", percentage * 100);
        generateBmp("/tmp/render.bmp", (char*) output, x, y);
        sleep(1);
    }
    for (int i = 0; i < num_threads; i++) {
         pthread_join(threads[i], NULL);
    }
    generateBmp("render.bmp", (char*) output, x, y);

}
