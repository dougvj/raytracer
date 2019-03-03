#include "trace.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "bmp.h"
#include <string.h>
#include "libdatastruct/linkedlist.h"


#define FOV_RADS 1.5708
typedef struct {
    vector p;
    vector n;
    material* m;
} intersection;

typedef struct {
    render_context* rc;
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
     int max_bounces;
     int rays_per_pixel;
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

    //origin
    double origin_x, origin_y, origin_z;
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
    //If the diffuse color is 0 that means we don't actually diffuse light
    if (diffuse_color == (vector){0} || COMPONENT(emission_color).w == 1)
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
    ;    *closest = *to_compare;
    }
}

//Here is where all the magic happens
void _traceRay(render_context* c, long pixel) {
    //Grab the ray and color that we are currently dealing with
    ray_color_pair* p = &(c->ray_color_pairs[pixel]);
    //If we have a 0 vector return
    if (p->r.d == (vector){0})
        return;
    //Create a new intersection object initialized to no hit that will
    //hold the closest intersection
    intersection closest = noHit();
    //Create a new intersection object which will be used to test new intersections
    intersection test;
    //And use our distance compare
    float_t distance = FLOAT_T_MAX;
    int count = 0;
    int i;
    //Go through each sphere
    for (i = 0; i < c->num_spheres; i++, count++) {
        test = intersectSphere(c->spheres[i], p->r);
        if (isHit(test))
            compareClosest(p->r, &distance, &closest, &test);
    }
    //Go trhough each plane
    for (i = 0; i < c->num_planes; i++, count++) {
        test = intersectPlane(c->planes[i], p->r);
        if (isHit(test))
           compareClosest(p->r, &distance, &closest, &test);
    }
    //Go through each triangle
    for (i = 0; i < c->num_triangles; i++, count++) {
        test = intersectTriangle(c->triangles[i], p->r);
        if (isHit(test))
           compareClosest(p->r, &distance, &closest, &test);
    }
    //Now if our closest object is a hit, then we know we intersected something
    if (isHit(closest)) {
        //We need to calculate our diffuse color  
        vector diffuse_color = ZERO_VECTOR();
        if (!closest.m->c_diffuse == (vector){0})
            for (i = 0; i < c->num_spheres; i++, count++)
                diffuse_color += calculateDiffuse(closest.m->c_diffuse, c->spheres[i]->m.c_emissions, c->spheres[i]->p, closest.p, closest.n);
        vector emission_color = closest.m->c_emissions;
        p->c = (diffuse_color + emission_color) * p->m.c_reflect + p->c;
        p->m.c_reflect *= closest.m->c_reflect;
        p->m.c_diffuse = closest.m->c_diffuse;
        p->m.c_emissions = closest.m->c_emissions;
        //This is our next ray
        p->r = (ray){closest.p, reflect(p->r.d, closest.n)};
    }
    else {
        p->r = (ray){ZERO_VECTOR(), ZERO_VECTOR()};
    }
}

color convertFloatToColor(vector cor) {
    vector c = cor;
    color co;
    if (c.x > 1.0f)
        c.x = 1.0f;
    if (c.y > 1.0f)
        c.y = 1.0f;
    if (c.z > 1.0f)
        c.z = 1.0f;
    co.r = c.x * 256;
    co.g = c.y * 255;
    co.b = c.z * 255;
    return co;
}

static const material null_material = {
     S_SCALAR(1.0),
     S_ZERO_VECTOR(),
     S_ZERO_VECTOR(),
};

void _generateOriginRay(render_context* c, int x, int y) {
    float_t rx = (x / (float_t)c->x) * 2 - 1.0f;
    float_t ry = (((y / (float_t)c->y) * 2 - 1.0f) * (c->y / (float)c->x)) * -1;
    float_t rz = 1.0f / tan(c->fov / 2.0f);
    vector d = VEC3F(rx, ry, rz);
    ray r = {VEC3F(c->origin_x, c->origin_y, c->origin_z), d};
    c->ray_color_pairs[y * c->x + x] = (ray_color_pair){r, ZERO_VECTOR(), null_material};
}




long getNextBlock(render_context* rc) {
    pthread_mutex_lock(&rc->mutex);
    long b =  rc->current_block;
    rc->current_block++;
    if (b >= rc->total_blocks ) {
        rc->cur_itr++;
        if (rc->cur_itr >= rc->max_bounces)
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
    long block = getNextBlock(c->rc);
    while (block != -1) {
        long start = c->rc->block_size * block;
        long end = c->rc->block_size * (block + 1);
        for (long pixel = start; pixel < end; pixel++) {
            _traceRay(c->rc, pixel);
            c->count_complete++;
        }
        declareBlockFinished(c->rc, block);
        block = getNextBlock(c->rc);
    }
}

render_context* createrender_context() {
    render_context* rc = aligned_malloc(64, sizeof(render_context));
    rc->ll_planes = llCreate();
    rc->ll_spheres = llCreate();
    rc->ll_triangles = llCreate();
    rc->ray_color_pairs = NULL;
    rc->fov = FOV_RADS;
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

typedef struct {
     render_context* rc;
     int start_x;
     int end_x;
     int start_y;
     int end_y;
     long count;
} _originRayParams;

void _generateOriginRayThread(_originRayParams* params) {
    for (int x = params->start_x; x < params->end_x; x++) {
        for (int y = params->start_y; y < params->end_y; y++) {
            _generateOriginRay(params->rc, x, y);
            params->count++;
        }
    }
}

void renderScene(render_context* rc, render_parameters* params) {
    rc->current_block = 0;
    rc->origin_x = params->origin_x;
    rc->origin_y = params->origin_y;
    rc->origin_z = params->origin_z;
    rc->x = params->x;
    rc->y = params->y;
    rc->num_threads = params->num_threads;
    rc->block_size = (rc->x * rc->y) / (params->num_threads * 10);
    rc->max_bounces = params->max_bounces;
    rc->total_blocks = params->num_threads * 10;
    rc->cur_itr = 0;
    int x = params->x;
    int y = params->y;
    //Generate arrays
    rc->ray_color_pairs = aligned_malloc(64, sizeof(ray_color_pair) * (x * y));
    rc->planes = (plane**)llCreateArray(rc->ll_planes);
    rc->num_planes = llGetCount(rc->ll_planes);
    rc->spheres = (sphere**)llCreateArray(rc->ll_spheres);
    rc->num_spheres = llGetCount(rc->ll_spheres);
    rc->triangles = (triangle**)llCreateArray(rc->ll_triangles);
    rc->num_triangles = llGetCount(rc->ll_spheres);
    rc->num_rays = x * y;
    printf("Generating origin rays\n");
    int num_blocks = params->num_threads * params->num_threads;
    _originRayParams ray_params[num_blocks];

    int block_size_y = (y / params->num_threads) + 1;
    int block_size_x = (x / params->num_threads) + 1;
    for (int i = 0; i < params->num_threads; i++) {
        for (int j = 0; j < params->num_threads; j++) {
             long o = (i * params->num_threads + j);
             ray_params[o].rc = rc;
             ray_params[o].start_x = j * block_size_x;
             ray_params[o].end_x = (j + 1) * block_size_x;
             ray_params[o].start_y = i * block_size_y;
             ray_params[o].end_y = (i + 1) * block_size_y;
             if (ray_params[o].end_y > y)
                 ray_params[o].end_y = y;
             if (ray_params[o].end_x > x)
                 ray_params[o].end_x = x;
             ray_params[o].count = 0;
        }
    }
    pthread_t threads[num_blocks];
    for (int i = 0; i < num_blocks; i++) {
        pthread_create(&(threads[i]), NULL, (void* (*)(void*)) _generateOriginRayThread, (void*)&ray_params[i]);
    }
    long count = 0;
    long size = x * y;
    while(count < size) {
        count = 0;
        for (int i = 0; i < num_blocks; i++) {
             count += ray_params[i].count;
        }
        double percentage = count/(double)size;
        fprintf(stderr, "%lf%% Origin Rays Complete\n", percentage * 100);
        sleep(1);
    }
    for (int i = 0; i < num_blocks; i++) {
         pthread_join(threads[i], NULL);
    }

    color* output;
    output = aligned_malloc(64, sizeof(color) * (x * y));
    rc->output = output;
    thread_context contexts[params->num_threads];
    for (int i = 0; i < params->num_threads; i++) {
        thread_context* c = &contexts[i];
        c->rc = rc;
        c->thread_num = i;
        c->count_complete = 0;
        c->num_traces = 0;
        pthread_create(&(threads[i]), NULL, (void *(*)(void*))_startRenderThread, (void*)c);
    }
    long complete = 0;
    long total = x * y * rc->max_bounces;
    double last_percentage = 0;
    long last_complete = 0;
    while (complete < total) {
        sleep(1);
        complete = 0;
        for (int i = 0; i < params->num_threads; i++) {
            complete += contexts[i].count_complete;
        }
        long complete_difference = complete - last_complete;
        double percentage = complete/(double)total;
        double difference = percentage - last_percentage;
        last_percentage = percentage;
        last_complete = complete;
        if (difference > 0) {
             double estimated_secs = ((1.0 / (difference)));
             double estimated_mins = estimated_secs / 60.0;
             long hours = estimated_mins / 60.0;
             long mins = (long)estimated_mins % 60;
	         long secs = (long)estimated_secs;
             fprintf(stderr, "%lf%% Complete ETA %li hr, %li min, %li secs, %li rays per sec\n", percentage * 100, hours, mins, secs, complete_difference);
        }
        else
            fprintf(stderr, "%lf%% Complete\n", percentage * 100);
    }
    for (int i = 0; i < params->num_threads; i++) {
         pthread_join(threads[i], NULL);
    }
    for (int i = 0; i < x; i++)
        for (int j = 0; j < y; j++)
            output[j * x + i] = convertFloatToColor(rc->ray_color_pairs[j * x + i].c);
    char filename[256];
    sprintf(filename, "render_output/%d.bmp", params->frame);
    generateBmp(filename, (char*) output, x, y);
    //render cleanup
    free(rc->planes);
    free(rc->spheres);
    free(rc->triangles);
    free(rc->ray_color_pairs);
}
