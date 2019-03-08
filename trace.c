#include "trace.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "libdatastruct/linkedlist.h"

#define THREAD_BLOCK_SIZE 32 //square pixels
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
    long num_rays;
    //output buffer
    color* output;
    pthread_mutex_t mutex;
    long current_block;
    int block_size;
    long block_stride;
    long total_blocks;
} render_context_t;

intersection noHit() {
    return (intersection){{0}, {0}, NULL};
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
    float_t b = dot(2.0 * r.d, rs);
    float_t c = dot(rs, rs) - s->r*s->r;
    float_t disc = (b*b) - (4 * a * c);
    if (disc >= 0) {
        float_t t = ((-b) - sqrt(disc))/(2*a);
        vector p = V3(X(r.p) + t * X(r.d), Y(r.p) + t * Y(r.d), Z(r.p) + t * Z(r.d));
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
    vector pos =  (r.p + r1 * r.d);
    material* m;
    if (abs((int)(floor(X(pos) / 5))) % 2 ==  abs((int)(floor(Z(pos) / 5))) % 2 )
        m = &p->m1;
    else
        m = &p->m2;
    return Hit(pos, normalize(p->n), m);
}

intersection intersectTriangle(triangle* t, ray r) {
    return noHit();

}

vector getBackground(ray r) {
    return V4(0.0f, 0.0f, 0.0f, 0.0f);
}

vector calculateDiffuse(vector diffuse_color, 
                        vector emission_color, 
                        vector light_pos, 
                        vector surface_pos, 
                        vector normal) {
    //If the diffuse color is 0 that means we don't actually diffuse light
    if (IS_VZERO(diffuse_color) || W(emission_color) == 1)
        return (vector){0};
    vector light = (light_pos - surface_pos);
    float_t distance = length(light);
    float_t square = 1 / (distance * distance);
    if (square * W(emission_color) < 0.01)
        return (vector){0};
    float_t incidence = dot(normalize(light), normalize(normal));
    if (incidence < 0)
        return (vector){0};
//        incidence *= -1;
    vector cor = emission_color;
    X(cor) *= W(cor);
    Y(cor) *= W(cor);
    Z(cor) *= W(cor);
    W(cor) = 0;
    cor = (cor * diffuse_color) * square * incidence;// * SCALAR(incidence));
    return cor;
}

void compareClosest(ray r, float_t* distance, intersection* closest, intersection* to_compare) {
    float_t new_distance = length((to_compare->p - r.p));
    if (new_distance < *distance ) {
        *distance = new_distance;
        *closest = *to_compare;
    }
}

static const material null_material = {
     V4(1.0, 1.0, 1.0, 1.0),
    {0},
    {0},
};

color toneMapFloatToColor(vector cor) {
    vector c = cor;
    color co;
    //Tone map 
    for (int i = 0; i < 4; i++)
     	c[i] = c[i] / (c[i] + 1);
    //Compress gamma
    for (int i = 0; i < 4; i++)
        c[i] = pow(c[i], 0.45454545);
    co.r = X(c) * 255;
    co.g = Y(c) * 255;
    co.b = Z(c) * 255;
    return co;
}



ray_color_pair _generateOriginRay(render_context* c, int x, int y) {
    float_t rx = (x / (float_t)c->x) * 2 - 1.0f;
    float_t ry = (((y / (float_t)c->y) * 2 - 1.0f) * (c->y / (float)c->x)) * -1;
    float_t rz = 1.0f / tan(c->fov / 2.0f);
    vector d = V3(rx, ry, rz);
    ray r = {V3(c->origin_x, c->origin_y, c->origin_z), d};
    return (ray_color_pair){r, {0}, null_material};
}

//Here is where all the magic happens
void _traceRay(render_context* c, int x, int y, int max_bounces) {
    //Generate the origin ray
    ray_color_pair p = _generateOriginRay(c, x, y);
    //fprintf(stderr, "%ix%i\n", x, y);
    //Grab the ray and color that we are currently dealing with
    for (int n = 0; n < max_bounces; n++) {
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
            test = intersectSphere(c->spheres[i], p.r);
            if (isHit(test))
                compareClosest(p.r, &distance, &closest, &test);
        }
        //Go trhough each plane
        for (i = 0; i < c->num_planes; i++, count++) {
            test = intersectPlane(c->planes[i], p.r);
            if (isHit(test))
               compareClosest(p.r, &distance, &closest, &test);
        }
        //Go through each triangle
        for (i = 0; i < c->num_triangles; i++, count++) {
            test = intersectTriangle(c->triangles[i], p.r);
            if (isHit(test))
               compareClosest(p.r, &distance, &closest, &test);
        }
        //Now if our closest object is a hit, then we know we intersected something
        if (isHit(closest)) {
            //We need to calculate our diffuse color
            vector diffuse_color = (vector) {0};
            if (!IS_VZERO(closest.m->c_diffuse))
                for (i = 0; i < c->num_spheres; i++, count++)
                    diffuse_color += calculateDiffuse(closest.m->c_diffuse, c->spheres[i]->m.c_emissions, c->spheres[i]->p, closest.p, closest.n);
            vector emission_color = closest.m->c_emissions;
            p.c = (diffuse_color + emission_color) * p.m.c_reflect + p.c;
            p.m.c_reflect *= closest.m->c_reflect;
            p.m.c_diffuse = closest.m->c_diffuse;
            p.m.c_emissions = closest.m->c_emissions;
            //This is our next ray
            p.r = (ray){closest.p, reflect(p.r.d, closest.n)};
        }
        else {
            p.r = (ray){{0}, {0}};
            break;
        }
    }
    //Output the pixel into the buffer
    long pixel = x + y * c->x;
    c->output[pixel] = toneMapFloatToColor(p.c);
}


#define NO_MORE_BLOCKS -1

long getNextBlock(render_context* rc) {
    pthread_mutex_lock(&rc->mutex);
    long b =  rc->current_block++;
    if (b >= rc->total_blocks ) {
        b = NO_MORE_BLOCKS;
    }
    pthread_mutex_unlock(&rc->mutex);
    return b;
}

void declareBlockFinished(render_context* rc, long block)
{

}

void _startRenderThread(thread_context* c) {
    long block = getNextBlock(c->rc);
    while (block != NO_MORE_BLOCKS) {
        //printf("Thread %i picking up block %li\n", c->thread_num, block);
        const long start_x = (block % c->rc->block_stride) * c->rc->block_size;
        const long start_y = (block / c->rc->block_stride) * c->rc->block_size;
        int end_x = start_x + c->rc->block_size;
        int end_y = start_y + c->rc->block_size;
        if (end_x > c->rc->x)
            end_x = c->rc->x;
        if (end_y > c->rc->y)
            end_y = c->rc->y;
        for (int x = start_x; x < end_x; x++) {
            for (int y = start_y; y < end_y; y++) {
                _traceRay(c->rc, x, y, c->rc->max_bounces);
                c->count_complete++;
             }
        }
        declareBlockFinished(c->rc, block);
        block = getNextBlock(c->rc);
    }
    //printf("Thread %i terminated, no more blocks\n", c->thread_num);
}

render_context* createRenderContext() {
    render_context* rc = aligned_malloc(64, sizeof(render_context));
    rc->ll_planes = llCreate();
    rc->ll_spheres = llCreate();
    rc->ll_triangles = llCreate();
    rc->fov = FOV_RADS;
    pthread_mutex_init(&rc->mutex, NULL);
    return rc;
}


void adjustMaterialGamma(material* m) {
    //Decode gamma to linera
    for (int i = 0; i < 3; i++)
        m->c_emissions[i] = pow(m->c_emissions[i], 2.2);
}

void addSphere(render_context* rc, sphere* s) {
    adjustMaterialGamma(&(s->m));
    llPushBack(rc->ll_spheres, s);
}

void addPlane(render_context* rc, plane* p) {
    adjustMaterialGamma(&(p->m1));
    adjustMaterialGamma(&(p->m2));
    llPushBack(rc->ll_planes, p);
}

void addTriangle(render_context* rc, triangle* t) {
    adjustMaterialGamma(&(t->m));
    llPushBack(rc->ll_triangles, t);
}

void renderScene(render_context* rc, render_parameters* params) {
    rc->current_block = 0;
    rc->origin_x = params->origin_x;
    rc->origin_y = params->origin_y;
    rc->origin_z = params->origin_z;
    rc->x = params->x;
    rc->y = params->y;
    rc->num_threads = params->num_threads;
    rc->max_bounces = params->max_bounces;
    rc->cur_itr = 0;
    int x = params->x;
    int y = params->y;
    //Generate arrays
    rc->planes = (plane**)llCreateArray(rc->ll_planes);
    rc->num_planes = llGetCount(rc->ll_planes);
    rc->spheres = (sphere**)llCreateArray(rc->ll_spheres);
    rc->num_spheres = llGetCount(rc->ll_spheres);
    rc->triangles = (triangle**)llCreateArray(rc->ll_triangles);
    rc->num_triangles = llGetCount(rc->ll_spheres);
    rc->num_rays = x * y;
    rc->output = (color*) params->output_buffer;
    rc->block_size = THREAD_BLOCK_SIZE;
    rc->block_stride = x / rc->block_size + !!(x % rc->block_size);
    rc->total_blocks = ((y / rc->block_size) + !!(y % rc->block_size)) * rc->block_stride;
    pthread_t threads[params->num_threads];
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
    long total = x * y;
    double last_percentage = 0;
    long last_complete = 0;
    while (complete < total) {
        usleep(250000);
        complete = 0;
        for (int i = 0; i < params->num_threads; i++) {
            complete += contexts[i].count_complete;
        }
        long complete_difference = (complete - last_complete) * 4.0;
        double percentage = complete/(double)total;
        double difference = (percentage - last_percentage) * 4.0;
        last_percentage = percentage;
        last_complete = complete;
        if (difference > 0) {
             double estimated_secs = ((1.0 / (difference)));
             double estimated_mins = estimated_secs / 60.0;
             long hours = estimated_mins / 60.0;
             long mins = (long)estimated_mins % 60;
	         long secs = (long)estimated_secs % 60;
             fprintf(stderr, "%lf%% Complete ETA %li hr, %li min, %li secs, %li pixels per sec\n", percentage * 100, hours, mins, secs, complete_difference);
        }
        else {
            fprintf(stderr, "%lf%% Complete\n", percentage * 100);
            break;
        }
    }
    for (int i = 0; i < params->num_threads; i++) {
         pthread_join(threads[i], NULL);
    }
    //render cleanup
    free(rc->planes);
    free(rc->spheres);
    free(rc->triangles);
}
