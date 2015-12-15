#include "trace.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "bmp.h"

#define MAX_BOUNCE 20
#define DIFFUSE_RES 30

typedef struct {
    int hit;
    float4 n;
    float4 p;
    material* m;
} intersection;

intersection noHit() {
    return (intersection){0, FLOAT4_Zero(), FLOAT4_Zero(), NULL};
}

intersection Hit(float4 pos, float4 norm, material* m) {
    return (intersection){1, norm, pos, m};
}

intersection intersectSphere(sphere* s, ray* r) {
    if (dot(r->d, FLOAT4_v(s->p.v - r->p.v)) <= 0)
       return noHit();
    float a = dot(r->d, r->d);
    float4 rs = FLOAT4_v(r->p.v - s->p.v);
    float b = dot(FLOAT4_v(FLOAT4_f(2.0f).v * r->d.v), rs);
    float c = dot(rs, rs) - s->r*s->r;
    float disc = (b*b) - (4 * a * c);
    //print_float4(r->d);
    if (disc >= 0) {
//        fprintf(stderr, "%f\n", disc);
        float t = ((-b) - sqrt(disc))/(2*a);
        float4 p = FLOAT4_3f(r->p.x + t * r->d.x, r->p.y + t * r->d.y, r->p.z + t * r->d.z);
        float4 n = FLOAT4_v(s->p.v - p.v);
        return Hit(p, n, &s->m);
    }
    return noHit();
}

intersection intersectPlane(plane* p, ray* r) {
    float n = dot(p->n, FLOAT4_v(p->p.v -  r->p.v));
    float d = dot(p->n, FLOAT4_v(-r->d.v));
    if (d == 0)
        return noHit();
    float r1 = n/d;
    //printf("%f\n", r1);
    if (r1 <= 1)
        return noHit();
    float4 pos =  FLOAT4_v(r->p.v + FLOAT4_f(r1).v * r->d.v);
    return Hit(pos, p->n, &p->m);
}

intersection intersectTriangle(triangle* t, ray* r) {
    return noHit();

}

intersection intersectEntity(entity* e, ray* r) {
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

float4 _traceRay(render_context* c, ray r, int num);

float4 findDiffuse(render_context* c, float4 p, float4 n, int num) {
    /*float deg_x = atan(n.x/n.y);
    float deg_z = atan(n.z/n.y);
    float sinx = sin(deg_x);
    float cosx = cos(deg_x);
    float sinz = sin(deg_z);
    float cosz = cos(deg_z);


    for (int i = 0; i < DIFFUSE_RES; i++) {
        float deg_p = (i / (float)DIFFUSE_RES) * (M_PI);
        int max_j = cos(deg_p) * (DIFFUSE_RES * 4);
        float cos_p = cos(deg_p);
        float sin_p = sin(deg_p);
        for (int j = 0; j < max_j; j++) {
            float deg_t = (j / (float)max_j) * (M_PI * 4);
            float x = sin(deg_t);
            float z = cos(deg_t);
            float4 v = FLOAT4_3f(x - (sin_p * x) , cos_p, z - (sin_p * z));
            v = FLOAT4_3f(v.x * cosz, v.y * sinz, v.z * cosx)
            float4 cor = _traceRay(c, (ray){p, v}, num + 1)
        }
    }*/
    return FLOAT4_Zero();
}

float4 getBackground(ray r) {
    return FLOAT4_4f(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 _traceRay(render_context* c, ray r, int num) {
    if (num > MAX_BOUNCE)
        return getBackground(r);
    LL_itr* itr = llInitIterator(c->entities);
    entity* e = (entity*)llGetNext(itr);
    intersection closest = noHit();
    float distance;
    while(e) {
        intersection i = intersectEntity(e, &r);
        if (i.hit) {
            float new_distance = length(FLOAT4_v(i.p.v - r.p.v));
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
        float4 reflect_color = _traceRay(c, (ray){closest.p, reflect(r.d, closest.n)}, num + 1);
        float4 diffuse_color = findDiffuse(c, closest.p, closest.n, num);
        return FLOAT4_v(closest.m->c_reflect.v * reflect_color.v +
               closest.m->c_diffuse.v * diffuse_color.v +
               closest.m->c_emissions.v);
    }
    return getBackground(r);
}

color convertFloatToColor(float4 c) {
//    print_float4(c);
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
//    fprintf(stderr, "color = {%u, %u, %u}\n", co.r, co.g, co.b);
    return co;
}

color _renderPixel(render_context* c, int x, int y) {
    float rx = (x / (float)c->x) * 2 - 1.0f;
    float ry = ((y / (float)c->y) * 2 - 1.0f) * (c->y / (float)c->x);
    float rz = 1.0f / tan(c->fov / 2.0f);
    float4 d = FLOAT4_3f(rx, ry, rz);
//    print_float4(d);
    ray r = {FLOAT4_3f(0.0f, 0.0f, 0.0f), d};
    return convertFloatToColor(_traceRay(c, r, 0));
}


typedef struct {
    render_context* c;
    int thread_num;
    int count_complete;
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
    render_context* rc = malloc(sizeof(render_context));
    rc->entities = llCreate();
    rc->fov = 1.5708f;
    return rc;
}


entity* createSphere(float4 pos, float r, material m) {
    sphere* s = malloc(sizeof(sphere));
    s->p = pos;
    s->r = r;
    entity* e = malloc(sizeof(entity));
    e->s = s;
    e->type = SPHERE;
    e->s->m = m;
    return e;
}

entity* createPlane(float4 pos, float4 norm, material m) {
    plane* p = malloc(sizeof(plane));
    p->p = pos;
    p->n = norm;
    entity* e = malloc(sizeof(entity));
    e->p = p;
    e->type = PLANE;
    e->p->m = m;
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
    output = malloc(sizeof(color) * (x * y));
    rc->output = output;
    pthread_t threads[num_threads];
    thread_context contexts[num_threads];
    for (int i = 0; i < num_threads; i++) {
        thread_context* c = &contexts[i];
        c->c = rc;
        c->thread_num = i;
        c->count_complete = 0;
        pthread_create(&(threads[i]), NULL, (void *(*)(void*))_startRenderThread, (void*)c);
    }
    long complete = 0;
    long total = x * y;
    while (complete < total) {
        complete = 0;
        for (int i = 0; i < num_threads; i++) {
            complete += contexts[i].count_complete;
        }
        float percentage = complete/(float)total;
        fprintf(stderr, "%4.2f%% Complete\n", percentage * 100);
        sleep(1);
    }
    for (int i = 0; i < num_threads; i++) {
         pthread_join(threads[i], NULL);
    }
    generateBmp("render.bmp", (char*) output, x, y);

}
