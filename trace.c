#include "trace.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "bmp.h"
#include <string.h>
#include "libdatastruct/linkedlist.h"


#define FOV_RADS 1.5708;
typedef struct {
    vector p;
    vector n;
    material* m;
} intersection;

typedef struct {
    render_context* c;
    int thread_num;
    int count_complete;
    int num_traces;
} thread_context;

typedef struct {
    ray r;
    vector c;
    material m;
} ray_color_pair;

struct render_context_t {
     int num_threads;
     int x;
     int y;
     int max_iterations;
     int cur_itr;
     float_t fov;

    //Primitives
    LL* ll_planes;
    plane** planes;
    int num_planes;
    LL* ll_spheres;
    sphere** spheres;
    int num_spheres;
    LL* ll_triangles;
    triangle** triangles;
    int num_triangles;

    //rays
    ray_color_pair* ray_color_pairs;
    long num_rays;

     color* output;
     pthread_mutex_t mutex;
     long current_block;
     long block_size;
     long total_blocks;
} render_context_t;

intersection noHit() {
    return (intersection){ZERO_VECTOR(), ZERO_VECTOR(), NULL};
}

intersection Hit(vector pos, vector norm, material* m) {
    return (intersection){pos, norm, m};
}

static inline int isHit(intersection i) {
    return i.m != NULL;
}

intersection intersectSphere(sphere* s, ray r){
    vector rs = s->p - r.p;
    if (dot(r.d, rs) <= 0)
       return noHit();
    float_t a = dot(r.d, r.d);
    rs = -rs;
    float_t b = dot(SCALAR(2.0) * r.d, rs);
    float_t c = dot(rs, rs) - s->r*s->r;
    float_t disc = (b*b) - (4 * a * c);
    if (disc >= 0) {
        float_t t = ((-b) - sqrt(disc))/(2*a);
        vector p = VEC3F(COMPONENT(r.p).x + t * COMPONENT(r.d).x, COMPONENT(r.p).y + t * COMPONENT(r.d).y, COMPONENT(r.p).z + t * COMPONENT(r.d).z);
        vector n = p - s->p;
        return Hit(p, normalize(n), &s->m);
    }
    return noHit();
}

intersection intersectPlane(plane* p, ray r) {
    float_t n = dot(p->n, (p->p -  r.p));
    float_t d = dot(p->n, r.d);
    if (d == 0)
        return noHit();
    float_t r1 = n/d;
    if (r1 <= 1)
        return noHit();
    vector pos =  (r.p + SCALAR(r1) * r.d);
    material* m;
    if (abs((int)(floor(COMPONENT(pos).x / 5))) % 2 ==  abs((int)(floor(COMPONENT(pos).z / 5))) % 2 )
        m = &p->m1;
    else
        m = &p->m2;
    return Hit(pos, normalize(p->n), m);
}

intersection intersectTriangle(triangle* t, ray r) {
    return noHit();

}

vector getBackground(ray r) {
    return VEC4F(0.0f, 0.0f, 0.0f, 0.0f);
}

vector calculateDiffuse(vector diffuse_color, vector emission_color, vector light_pos, vector surface_pos, vector normal) {
    if (isZero(diffuse_color) || COMPONENT(emission_color).w == 0)
        return ZERO_VECTOR();
    vector light = (surface_pos - light_pos);
    float_t distance = length(light);
    float_t square = 1 / (distance * distance);
    if (square * COMPONENT(emission_color).w < 0.01)
        return ZERO_VECTOR();
    float_t incidence = dot(normalize(light), normalize(normal));
    if (incidence < 0)
        incidence *= -1;
    vector_accessor cor_a = COMPONENT(emission_color);
    cor_a.x *= cor_a.w;
    cor_a.y *= cor_a.w;
    cor_a.z *= cor_a.w;
    cor_a.w = 0;
    vector cor = cor_a.v;
    cor = (cor * diffuse_color) * SCALAR(square) * SCALAR(incidence);// * SCALAR(incidence));
    return cor;
}

void compareClosest(ray r, float_t* distance, intersection* closest, intersection* to_compare) {
    float_t new_distance = length((to_compare->p - r.p));
    if (new_distance < *distance ) {
        *distance = new_distance;
        *closest = *to_compare;
    }
}

void _traceRay(render_context* c, long pixel) {
    ray_color_pair* p = &(c->ray_color_pairs[pixel]);
    if (isZero(p->r.d))
        return;
    intersection closest = noHit();
    intersection test;
    float_t distance = FLOAT_T_MAX;
    int count = 0;
    int i;
    for (i = 0; i < c->num_spheres; i++, count++) {
        test = intersectSphere(c->spheres[i], p->r);
        if (isHit(test))
            compareClosest(p->r, &distance, &closest, &test);
    }
    for (i = 0; i < c->num_planes; i++, count++) {
        test = intersectPlane(c->planes[i], p->r);
        if (isHit(test))
           compareClosest(p->r, &distance, &closest, &test);
    }
    for (i = 0; i < c->num_triangles; i++, count++) {
        test = intersectTriangle(c->triangles[i], p->r);
        if (isHit(test))
           compareClosest(p->r, &distance, &closest, &test);
    }
    if (isHit(closest)) {
        vector diffuse_color = ZERO_VECTOR();
        if (!isZero(closest.m->c_diffuse))
            for (i = 0; i < c->num_spheres; i++, count++)
                diffuse_color += calculateDiffuse(closest.m->c_diffuse, c->spheres[i]->m.c_emissions, c->spheres[i]->p, closest.p, closest.n);
        vector emission_color = closest.m->c_emissions;
        p->c = (diffuse_color + emission_color) * p->m.c_reflect + p->c;
        p->m.c_reflect *= closest.m->c_reflect;
        p->m.c_diffuse = closest.m->c_diffuse;
        p->m.c_emissions = closest.m->c_emissions;
        p->r = (ray){closest.p, reflect(p->r.d, closest.n)};
    }
    else {
        p->r = (ray){ZERO_VECTOR(), ZERO_VECTOR()};
    }
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

static const material null_material = {
    SCALAR(1.0),
     ZERO_VECTOR(),
     ZERO_VECTOR(),
};

void _generateOriginRay(render_context* c, int x, int y) {
    float_t rx = (x / (float_t)c->x) * 2 - 1.0f;
    float_t ry = (((y / (float_t)c->y) * 2 - 1.0f) * (c->y / (float)c->x)) * -1;
    float_t rz = 1.0f / tan(c->fov / 2.0f);
    vector d = VEC3F(rx, ry, rz);
    ray r = {VEC3F(0.0f, 0.0f, 0.0f), d};
    c->ray_color_pairs[y * c->x + x] = (ray_color_pair){r, ZERO_VECTOR(), null_material};
}




long getNextBlock(render_context* rc) {
    pthread_mutex_lock(&rc->mutex);
    long b =  rc->current_block;
    rc->current_block++;
    if (b >= rc->total_blocks ) {
        rc->cur_itr++;
        if (rc->cur_itr >= rc->max_iterations)
            b = -1;
        else {
            b = 0;
            rc->current_block = 0;
        }
    }
    pthread_mutex_unlock(&rc->mutex);
    return b;
}

void declareBlockFinished(render_context* rc, long block)
{

}

void _startRenderThread(thread_context* c) {
    long block = getNextBlock(c->c);
    while (block != -1) {
        long start = c->c->block_size * block;
        long end = c->c->block_size * (block + 1);
        for (long pixel = start; pixel < end; pixel++) {
            _traceRay(c->c, pixel);
            c->count_complete++;
        }
        declareBlockFinished(c->c, block);
        block = getNextBlock(c->c);
    }
}

render_context* createRenderContext() {
    render_context* rc = aligned_malloc(64, sizeof(render_context));
    rc->ll_planes = llCreate();
    rc->ll_spheres = llCreate();
    rc->ll_triangles = llCreate();
    rc->fov = FOV_RADS;
    rc->current_block = 0;
    pthread_mutex_init(&rc->mutex, NULL);
    return rc;
}



void addSphere(render_context* rc, sphere* s) {
    llPushBack(rc->ll_spheres, s);
}

void addPlane(render_context* rc, plane* p) {
    llPushBack(rc->ll_planes, p);
}

void addTriangle(render_context* rc, triangle* t) {
    llPushBack(rc->ll_triangles, t);
}


void renderScene(render_context* rc, int x, int y, int num_threads, int max_iterations) {
    rc->x = x;
    rc->y = y;
    rc->num_threads = num_threads;
    rc->block_size = (x * y) / (num_threads * 10);
    rc->max_iterations = max_iterations;
    rc->total_blocks = num_threads * 10;
    rc->cur_itr = 0;
    //Generate arrays
    rc->planes = (plane**)llCreateArray(rc->ll_planes);
    rc->num_planes = llGetCount(rc->ll_planes);
    rc->spheres = (sphere**)llCreateArray(rc->ll_spheres);
    rc->num_spheres = llGetCount(rc->ll_spheres);
    rc->triangles = (triangle**)llCreateArray(rc->ll_triangles);
    rc->num_triangles = llGetCount(rc->ll_spheres);
    rc->ray_color_pairs = aligned_malloc(64, sizeof(ray_color_pair) * (x * y));
    rc->num_rays = x * y;
    for (int i = 0; i < x; i++)
        for (int j = 0; j < y; j++)
            _generateOriginRay(rc, i, j);
    color* output;
    output = aligned_malloc(64, sizeof(color) * (x * y));
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
    long total = x * y * rc->max_iterations;
    while (complete < total) {
        complete = 0;
        for (int i = 0; i < num_threads; i++) {
            complete += contexts[i].count_complete;
        }
        double percentage = complete/(double)total;
        fprintf(stderr, "%lf%% Complete\n", percentage * 100);
        sleep(1);
    }
    for (int i = 0; i < num_threads; i++) {
         pthread_join(threads[i], NULL);
    }
    for (int i = 0; i < x; i++)
        for (int j = 0; j < y; j++)
            output[j * x + i] = convertFloatToColor(rc->ray_color_pairs[j * x + i].c);
    generateBmp("render.bmp", (char*) output, x, y);

}
