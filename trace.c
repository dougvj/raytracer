#include "trace.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define THREAD_BLOCK_SIZE 128 //square pixels
#define FOV_RADS 1.5708
#define GAMMA 2.2
typedef struct {
    vector p;
    vector n;
    material* m;
} intersection;

typedef struct {
    render_context* rc;
    int thread_num;
    int count_complete;
    long num_traces;
    long num_intersections;
    int block;
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

    //Rendering flag
    int rendering;

    //Primitives
    plane* planes;
    int num_planes;
    sphere* spheres;
    int num_spheres;
    triangle* triangles;
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
        vector p = V3(
            X(r.p) + t * X(r.d), 
            Y(r.p) + t * Y(r.d), 
            Z(r.p) + t * Z(r.d)
        );
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
    .c_reflect = V4(1.0, 1.0, 1.0, 1.0),
    .c_diffuse = {0},
    .c_emit    = {0}
};

color toneMapFloatToColor(vector cor) {
    vector c = cor;
    color co;
    //Tone map 
    for (int i = 0; i < 4; i++)
     	c[i] = c[i] / (c[i] + 1);
    //Compress gamma
    for (int i = 0; i < 4; i++)
        c[i] = pow(c[i], 1.0/GAMMA);
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

typedef struct {
    int num_traces;
    int num_intersections;
} trace_stats;

//Here is where all the magic happens
trace_stats _traceRay(render_context* c, int x, int y, int max_bounces) {
    trace_stats tc = {0, 0};
    //Generate the origin ray
    ray_color_pair p = _generateOriginRay(c, x, y);
    //fprintf(stderr, "%ix%i\n", x, y);
    //Grab the ray and color that we are currently dealing with
    for (int n = 0; n < max_bounces; n++) {
        tc.num_traces++;
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
            tc.num_intersections++;
            test = intersectSphere(&c->spheres[i], p.r);
            if (isHit(test))
                compareClosest(p.r, &distance, &closest, &test);
        }
        //Go trhough each plane
        for (i = 0; i < c->num_planes; i++, count++) {
            tc.num_intersections++;
            test = intersectPlane(&c->planes[i], p.r);
            if (isHit(test))
               compareClosest(p.r, &distance, &closest, &test);
        }
        //Go through each triangle
        for (i = 0; i < c->num_triangles; i++, count++) {
            tc.num_intersections++;
            test = intersectPlane(&c->planes[i], p.r);
            printf("nt: %i", c->num_triangles);
            test = intersectTriangle(&c->triangles[i], p.r);
            if (isHit(test))
               compareClosest(p.r, &distance, &closest, &test);
        }
        //Now if our closest object is a hit, then we know we intersected something
        if (isHit(closest)) {
            //We need to calculate our diffuse color
            vector diffuse_color = (vector) {0};
            if (!IS_VZERO(closest.m->c_diffuse)) {
                for (i = 0; i < c->num_spheres; i++, count++) {
                    diffuse_color += calculateDiffuse(
                            closest.m->c_diffuse, 
                            c->spheres[i].m.c_emit, 
                            c->spheres[i].p, 
                            closest.p, 
                            closest.n
                    );
                }
            }
            vector emission_color = closest.m->c_emit;
            p.c = (diffuse_color + emission_color) * p.m.c_reflect + p.c;
            p.m.c_reflect *= closest.m->c_reflect;
            p.m.c_diffuse = closest.m->c_diffuse;
            p.m.c_emit = closest.m->c_emit;
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
    return tc;
}


#define NO_MORE_BLOCKS -1

long getNextBlock(render_context* rc) {
    pthread_mutex_lock(&rc->mutex);
    long b =  rc->current_block++;
    if (b >= rc->total_blocks || !rc->rendering) {
        b = NO_MORE_BLOCKS;
    }
    pthread_mutex_unlock(&rc->mutex);
    return b;
}

void declareBlockFinished(render_context* rc, long block) {

}



void _startRenderThread(thread_context* c) {
    long block = getNextBlock(c->rc);
    while (block != NO_MORE_BLOCKS) {
        c->block = block;
        //printf("Thread %i picking up block %li\n", c->thread_num, block);
        const long start_x = (block % c->rc->block_stride) * c->rc->block_size;
        const long start_y = (block / c->rc->block_stride) * c->rc->block_size;
        int end_x = start_x + c->rc->block_size;
        int end_y = start_y + c->rc->block_size;
        if (end_x > c->rc->x)
            end_x = c->rc->x;
        if (end_y > c->rc->y)
            end_y = c->rc->y;
        for (int y = start_y; y < end_y; y++) {
            for (int x = start_x; x < end_x; x++) {
                trace_stats tc = _traceRay(c->rc, x, y, c->rc->max_bounces);
                c->num_traces += tc.num_traces;
                c->num_intersections += tc.num_intersections;
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
    memset(rc, 0, sizeof(render_context));
    rc->fov = FOV_RADS;
    pthread_mutex_init(&rc->mutex, NULL);
    return rc;
}

void deleteRenderContext(render_context* rc) {
    pthread_mutex_destroy(&rc->mutex);
    free(rc);
}


void adjustMaterialGamma(material* m) {
    //Decode gamma to linera
    for (int i = 0; i < 3; i++)
        m->c_emit[i] = pow(m->c_emit[i], GAMMA);
}

static double _getTimestamp() {
    struct timespec curtime;
    clock_gettime(CLOCK_MONOTONIC, &curtime);
    return curtime.tv_sec + curtime.tv_nsec / 1000000000.0;
}

typedef struct {
    render_context* rc;
    int num_threads;
    thread_context* thread_contexts;
    window* w;
} window_thread_context;

#define MAX_FRAME_RATE 60
static void _windowThread(window_thread_context* wtc) {
    double frame_time = 1.0/MAX_FRAME_RATE;
    double last_frame = 0;
    long window_frames = 0;
    double fps_time = _getTimestamp();
    if (!windowSetDrawingThread(wtc->w)) {
        fprintf(stderr, "Cannot initialize drawing thread\n");
        return;
    }
    while(wtc->rc->rendering) {
        double next_frame = (frame_time - (_getTimestamp() - last_frame)) ;
        //Wait until next frame should be rendered
        double error = 0;
        if (next_frame > 0) {
            struct timespec t;
            t.tv_nsec = (next_frame) * 1000000000;
            t.tv_sec = 0;
            //Sleep and measure our wakeup time overshoot
            double s = _getTimestamp();
            nanosleep(&t, NULL);
            double e = _getTimestamp();
            //The eror is subtracted from the next frame time
            error = (e-s) - next_frame;
        } 
        //Current timestamp minus our overshoot
        last_frame = _getTimestamp() - error;
        if (last_frame - fps_time > 1) {
            fprintf(stderr,"FPS: %f, ERR: %f\n", window_frames / (last_frame - fps_time), error);
            fps_time = last_frame;
            window_frames = 0;
        }
        rect r = {
            .x = 0,
            .y = 0,
            .w = wtc->rc->x,
            .h = wtc->rc->y
        };
        rect rect_list[wtc->num_threads];
        for (int i = 0; i < wtc->num_threads; i++) {
            long block = wtc->thread_contexts[i].block;
            if (block >= 0) {
                long start_x = (block % wtc->rc->block_stride) 
                                            * wtc->rc->block_size;
                long start_y = (block / wtc->rc->block_stride) 
                                            * wtc->rc->block_size;
                rect_list[i] = (rect){
                    .x = start_x,
                    .y = start_y,
                    .w = wtc->rc->block_size,
                    .h = wtc->rc->block_size
                };
            }
        }
        windowDrawBuffer(wtc->w, (char*)wtc->rc->output, wtc->rc->x, wtc->rc->y, r);
        for(int i = 0; i < wtc->num_threads; i++) {
            windowDrawRect(wtc->w, rect_list[i]);
        }
        windowUpdate(wtc->w);
        if (windowCheckQuit(wtc->w)) {
            wtc->rc->rendering = 0;
        }
        window_frames++;
    }
    windowFreeDrawingThread(wtc->w);
} 

typedef struct {
    render_context* rc;
    thread_context* thread_contexts;
    int num_threads;
    long total_pixels;
    double start_time;
    double end_time;
} stats_context;


#define STATS_FREQ 16
static void _statsThread(stats_context* sc) {
    long total = sc->total_pixels;
    long complete = 0;
    long num_traces = 0;
    long last_num_traces = 0;
    do {
        usleep(1000000/STATS_FREQ);
        complete = 0;
        num_traces = 0;
        for (int i = 0; i < sc->num_threads; i++) {
            complete += sc->thread_contexts[i].count_complete;
            num_traces += sc->thread_contexts[i].num_traces;
        }
        long traces_per_sec = (num_traces - last_num_traces) * STATS_FREQ;
        double percentage = complete/(double)total;
        last_num_traces = num_traces;
        double total_secs = _getTimestamp() - sc->start_time;
        long msecs = (int)(total_secs * 1000.0) % 1000;
        long mins = total_secs / 60;
        long hours = mins / 60;
        mins %= 60;
        long secs = (long)(total_secs) % 60;
        long avg_traces_per_sec = num_traces / total_secs;
        fprintf(
            stderr, 
           "%10.6lf%% Complete. Elapsed %02li:%02li:%02li.%03li, cur: %10li avg: %10li traces per sec\r", 
           percentage * 100, hours, mins, secs, msecs, traces_per_sec, avg_traces_per_sec
        );
    } while(sc->rc->rendering);

    fprintf(stderr, "\n");
    num_traces = 0;
    long num_intersections = 0;
    for (int i = 0; i < sc->num_threads; i++) {
        num_traces += sc->thread_contexts[i].num_traces;
        num_intersections += sc->thread_contexts[i].num_intersections;
    }
    //End time is only set if we completed
    if (sc->end_time > 0) {
        //Display elapsed time and traces per sec
        double total_time = sc->end_time - sc->start_time;
        fprintf(stderr, 
                "Completed. Stats:\n"
                " Total Frame Render Time:        %10.4lfs\n"
                " Total Frame Traces:             %10.4lfM\n"
                " Traces Per Sec:                 %10.4lfK\n" 
                " Total Geometric Intersections:  %10.4lfG\n"
                " Geometric Intersections per Sec %10.4lfM\n",
                total_time,
                num_traces / 1000000.0,
                (long)(num_traces / total_time) / 1000.0,
                num_intersections / 1000000000.0,
               (num_intersections / total_time) / 1000000.0
        );
    }
}

int renderScene(render_context* rc, render_parameters* params) {
    //Record start time
    double start_time = _getTimestamp();
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
    //Copy params to render context
    rc->planes = params->planes;
    rc->num_planes = params->num_planes;
    rc->spheres = params->spheres;
    rc->num_spheres = params->num_spheres;
    rc->triangles = params->triangles;
    rc->num_triangles = params->num_triangles;
    rc->num_rays = x * y;
    rc->output = (color*) params->output_buffer;
    //Set block and stride information for render threads
    rc->block_size = THREAD_BLOCK_SIZE;
    rc->block_stride = x / rc->block_size + !!(x % rc->block_size);
    rc->total_blocks = ((y / rc->block_size) + !!(y % rc->block_size)) * rc->block_stride;
    //Create the worker threads eith their thread_contexts
    pthread_t threads[params->num_threads];
    thread_context thread_contexts[params->num_threads];
    //Flag that we are rendering
    rc->rendering = 1;
    for (int i = 0; i < params->num_threads; i++) {
        thread_context* c = &thread_contexts[i];
        c->rc = rc;
        c->thread_num = i;
        c->count_complete = 0;
        c->num_traces = 0;
        c->num_intersections = 0;
        c->block = -1;
        pthread_create(&(threads[i]), NULL, (void *(*)(void*))_startRenderThread, (void*)c);
    }
    //If we have  window parameter then we need to start a window thread
    pthread_t window_thread;
    window_thread_context wtc;
    if(params->w) {
        wtc = (window_thread_context){
            .rc = rc,
            .thread_contexts = thread_contexts,
            .w = params->w,
            .num_threads = params->num_threads
        };
        pthread_create(&window_thread, NULL, (void *(*)(void*))_windowThread, (void*)&wtc);
    }
    
    //Start the thread that displays stats
    pthread_t stats_thread;
    stats_context sc = {
        .thread_contexts = thread_contexts,
        .num_threads = params->num_threads,
        .total_pixels = x * y,
        .start_time = start_time,
        .end_time = 0,
        .rc = rc
    };
    pthread_create(&stats_thread, NULL, (void *(*)(void*))_statsThread, (void*)&sc);
    //Join the worker threads
    for (int i = 0; i < params->num_threads; i++) {
         pthread_join(threads[i], NULL);
    }
    int completed = 1;
    if (rc->rendering == 0) {
        completed = 0;
        fprintf(stderr, "Rendering was interrupted\n");
    }
    //Mark the final time for the stats thread
    sc.end_time = _getTimestamp();
    //We are no longer rendering
    rc->rendering = 0;
    //Join the window thread if we started it
    if (params->w) {
       pthread_join(window_thread, NULL); 
    }
    //Join the stats threads
    pthread_join(stats_thread, NULL);
    return completed;
}
