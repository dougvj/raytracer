#include <stdlib.h>
#include <stdio.h>
#include "trace.h"
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include "geometric_primitives.h"
//#include "window.h"
#include "bmp.h"

typedef struct {
    int w;
    int h;
    int num_threads;
    int num_frames;
    int max_bounces;
    //Path tracing options
    int path_tracing_enabled;
    int samples_per_pixel;
    //Window options
    int window_enabled;
    int win_w;
    int win_h;
    int random_seed;
    char* output_dir;
} program_parameters;


float frand() {
    return rand() / (float)RAND_MAX;
}

static const char usage_string[] = 
"\
General Options:\n\
    --resolution <width>x<height> Required output resolution argument, \n\
                                  ie 800x600\n\
    --num-threads <num>   Number of worker threads\n\
                          Default is machine size\n\
    --max-bounces <num>   The maximum number of ray traced bounces.\n\
                          The default is 8\n\
    --random-seed <num>   The seed for the srand()\n\
\n\
Path Tracing Options:\n\
    --enable_path_tracing Set to enable full monte-carlo path tracing\n\
                          For soft shadows, global illumination, etc\n\
    --samples_per_pixels  The number of samples per pixel for path tracing\n\
\n\
Output Options:\n\
    --enable-window       Enable output window for live viewing of output\n\
    --window-resolution <width>x<height> Resolution of output window\n\
                                         if enabled\n\
    --output-directory <dir> Output directory for the frames\n\
    --output-format <format> Output format for the frames, default bmp\n\
    --num-frames <num> Number of frames to geneate. Default 1\n\
";

static void print_usage_and_quit() {
    fprintf(stderr, "%s", usage_string);
    exit(1);
}

static program_parameters params;

static int parse_int(char* arg) {
    //TODO check valid integer with different function
    return atoi(arg);
}

static void parse_resolution_arg(char* arg, int* x, int* y) {
    char* res = arg;
    char* w = res;
    char* h;
    //find the 'x' part
    for (h = res; *h != '\0' && *h != 'x'; h++);
    if (*h == '\0') {
        fprintf(stderr, "Expected <width>x<height>\n"); 
        exit(1);
    }
    h++;
    *x = parse_int(w);
    *y = parse_int(h);
    fprintf(stderr, "Got %i, %i\n", *x, *y);
}

static void parse_args(int argc, char** argv) {
    for(;;) {
        //Declare our options scoped in for
        static struct option options[] = {
            //required arguments
            { //0
                .name = "resolution",
                .has_arg = required_argument 
            },
            //optional arguments with defaults
            { //1
                .name = "num-threads",
                .has_arg = required_argument
            },
            { //2
                .name = "max-bounces",
                .has_arg = required_argument
            },
            { //3
                .name = "enable-path-tracing",
                .flag = &(params.path_tracing_enabled),
                .val = 1
            },
            { //4
                .name = "samples-per-pixel",
                .has_arg = required_argument
            },
            { //5
                .name = "num-frames",
                .has_arg = required_argument
            },
            { //6
                .name = "enable-window",
                .flag = &(params.window_enabled),
                .val = 1
            },
            { //7
                .name = "window-resolution",
                .has_arg = required_argument
            },
            { //8
                .name = "output-directory",
                .has_arg = required_argument
            },
            { //9
                .name = "help",
            },
            { //10
                .name = "random-seed",
                .has_arg = required_argument
            },
        };
        int i;
        int c = getopt_long(argc, argv, "", options, &i);
        if (c == -1)
            break;
        switch(c) {
            case 0:
                /*printf("getopt returned %c 0x%X\n", c, (int) c);
                printf("option %s", options[i].name);
                if (options[i].flag != 0) {
                    printf(" is %s\n", (*(options[i].flag) ? "set": "not set"));
                } else {
                    printf(" with arg %s\n", optarg);
                } */
                switch(i) {
                    case 0:  //"res" parameter
                        parse_resolution_arg(optarg, &(params.w), &(params.h));
                        break;
                    case 1:  //num threads parameter
                        params.num_threads = parse_int(optarg);
                        break;
                    case 2: //max-bounces
                        params.max_bounces = parse_int(optarg);
                        break;
                    case 4: //samples-per-pixel
                        params.samples_per_pixel = parse_int(optarg);
                        break;
                    case 5: //num-frames
                        params.num_frames = parse_int(optarg);
                        break;
                    case 7: //window-resolution
                        parse_resolution_arg(optarg, &(params.win_w), &(params.win_h));
                        break;
                    case 8:
                        params.output_dir = optarg;//Need to dup?
                        break;
                    case 9: //help
                        print_usage_and_quit();
                        break;
                    case 10:
                        params.random_seed = parse_int(optarg);

                }
                break;
            default:
                print_usage_and_quit();
        }
    }
}

static void print_parameters() {
    fprintf(stderr, "Running with the following parameters:\n");
    fprintf(stderr, "\tOutput Resolution: %ix%i\n", params.w, params.h);
    fprintf(stderr, "\tWindowed Output: %s\n", params.window_enabled ? "True" : "False");
    if (params.window_enabled) {
        fprintf(stderr, "\t\tWindow Resolution: %ix%i\n", params.win_w, params.win_h);
    }
    fprintf(stderr, "\tOutput Directory: %s\n", params.output_dir);
    fprintf(stderr, "\tNum Frames: %i\n", params.num_frames);
    fprintf(stderr, "\tNum Worker Threads: %i\n", params.num_threads);
    fprintf(stderr, "\tMax Bounces: %i\n", params.max_bounces);
    fprintf(stderr, "\tPath Tracing: %s\n", params.path_tracing_enabled ? "True" : "False");
    if (params.path_tracing_enabled) {
        fprintf(stderr, "\tSamples per Pixel: %i\n", params.samples_per_pixel);
    }
    if (params.random_seed >= 0) {
        fprintf(stderr, "\tRandom Seed: %i\n", params.random_seed);
    } 
}

int main(int argc, char** argv) {
    //Set default parameter values
    //Most values are 0 by default
    memset(&params, 0, sizeof(program_parameters));
    //Set number of threads to system's hardware threads
    params.num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    //Sensible Defaults
    params.max_bounces = 8;
    params.num_frames = 1;
    params.random_seed = -1; //Indicates to use TIME()
    //Parse the arguments
    parse_args(argc, argv);
    //Validate parameters
    if (params.w == 0 || params.h == 0) {
        fprintf(stderr, "Output resolution not set with --resolution\n");
        exit(1);
    }
    if (!params.output_dir) {
        params.output_dir = "render_output";
    }
    print_parameters();
    //srand(100);
    if (params.random_seed)
        srand(params.random_seed);
    else
        srand(time(NULL));
    render_context* rc = createRenderContext();
    /*addSphere(rc,
            createSphere(V3(0, 100, 500), 1.0f,
                (material) {
                    V3(0.0, 0.0, 0.0),
                    V3(0.0, 0.0, 0.0),
                    V4(1.0f, 1.0f, 1.0f, 100000.0),
                }
            )
    );*/
    /*addSphere(rc,
            createSphere(V3(-20000.0, 100000.0, -20000.0), 100.0f,
                (material) {
                    V3(0.0, 0.0, 0.0),
                    V3(0.0, 0.0, 0.0),
                    V4(1.0f, 1.0f, 1.0f, 5000000000.0),
                }
            )
    );*/
    addSphere(rc,
            createSphere(V3(0.0, 4.0, 53.0), 2.3,
                (material) {
                    V3(1.0, 1.0, 1.0),
                    V3(0.2, 0.2, 0.2),
                    V4(0.05f, 0.05f, 0.05f, 0.0f),
                }
            )
    );
    addSphere(rc,
            createSphere(V3(0.0, 0.0, 55.2), 1.5,
                (material) {
                    V3(0.6, 0.6, 0.6),
                    V3(0.0, 0.0, 0.0),
                    V4(0.0, 0.0, 1.0, 100),
                }
            )
    );
    addSphere(rc,
            createSphere(V3(-3.0, 0.0, 50.0), 1.5,
                (material) {
                    V3(0.6, 0.6, 0.6),
                    V3(0.0, 0.0, 0.0),
                    V4(0.0, 1.0, 0.0, 100),
                }
            )
    );
    addSphere(rc,
            createSphere(V3(3.0, 0.0, 50.0), 1.5,
                (material) {
                    V3(0.6, 0.6, 0.6),
                    V3(0.0, 0.0, 0.0),
                    V4(1.0, 0.0, 0.0, 100.0),
                }
            )
    );
    addPlane(rc,
            createPlane(V3(0.0, -6.0, 0.0),
                        V3(0.0, 1.0, 0.0),
                (material) {
                    V3(0.1, 0.1, 0.1),
                    V3(0.28, 0.35, 0.35),
                    V4(0.0, 0.0, 0.0, 0.0),
                },
                (material) {
                    V3(0.25, 0.25, 0.25),
                    V3(0.04, 0.05, 0.05),
                    V4(0.0, 0.0, 0.0, 0.0f),
                }
            )
    );
   for (int i = 0; i < 3000; i++) {
        vector color = V3(0.8 * frand() + 0.1, 0.8 * frand() + 0.1, 0.8 * frand() + 0.1);
        addSphere(rc,
                createSphere(V3(frand() * 1000. - 500., frand() * 20 + 9.4,  frand() * 1000 - 200), frand() * 3 + 1,
                (material) {
                    .c_reflect = color,
                    .c_diffuse = color,
                    V4(0.0, 0.0, 0.0, 0.0),
                })
        );
    }
    //Allocate the output frame buffer
    char* output_buffer = aligned_malloc(64, 
                                         sizeof(char[3]) * params.w * params.h);
    render_parameters render_params = (render_parameters){
        .x = params.w,
        .y = params.h,
        .num_threads = params.num_threads,
        .max_bounces = params.max_bounces,
        .rays_per_pixel = params.samples_per_pixel,
        .origin_x = 0.0,
        .origin_y = 0.0,
        .output_buffer = output_buffer
        //origin_z and frame filled in loop

    };
    for (int i = 0; i < params.num_frames; i++) {
        render_params.origin_z = -10 + (i/10.0);
    	fprintf(stderr, "Generating frame %d\n", i);
    	renderScene(rc, &render_params);
        char filename[256];
        snprintf(filename, 256, "%s/%d.bmp", params.output_dir, i);
        printf("Dumping frame to %s...\n", filename);
        FILE* fh = fopen(filename, "wb");
        if (!fh) {
            fprintf(stderr, "Could not open file. Perhaps a permissions issue "
                            "or path not found?\n");
            exit(1);
            
        }
        generateBmp(fh, output_buffer, params.w, params.h);
        fclose(fh);
    }
    return 0;
}
