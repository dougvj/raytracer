#include <stdlib.h>
#include <stdio.h>
#include "trace.h"
#include <time.h>
#include "geometric_primitives.h"
//#include "window.h"
#include "bmp.h"

float frand() {
    return rand() / (float)RAND_MAX;
}

int main(int argc, char** argv) {
    if (argc < 7) {
        fprintf(stderr, "Usage:\n\traytracer <width> <height> <num_threads> <max_bounces> <rays_per_pixel> <num_frames>\n");
        return 1;
    }
    srand(100);
    //srand(time(NULL));
    int num_frames = atoi(argv[6]);
    int rays_per_pixel = atoi(argv[5]);
    int max_bounces = atoi(argv[4]);
    int num_threads = atoi(argv[3]);
    int w = atoi(argv[1]);
    int h = atoi(argv[2]);
    render_context* rc = createRenderContext();
    addSphere(rc,
            createSphere(V3(2000.0, 10000.0, -20000.0), 100.0f,
                (material) {
                    V3(0.0, 0.0, 0.0),
                    V3(0.0, 0.0, 0.0),
                    V4(1.0f, 1.0f, 1.0f, 200000000.0),
                }
            )
    );
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
                    V4(0.0, 0.0, 1.0, 60),
                }
            )
    );
    addSphere(rc,
            createSphere(V3(-3.0, 0.0, 50.0), 1.5,
                (material) {
                    V3(0.6, 0.6, 0.6),
                    V3(0.0, 0.0, 0.0),
                    V4(0.0, 1.0, 0.0, 60),
                }
            )
    );
    addSphere(rc,
            createSphere(V3(3.0, 0.0, 50.0), 1.5,
                (material) {
                    V3(0.6, 0.6, 0.6),
                    V3(0.0, 0.0, 0.0),
                    V4(1.0, 0.0, 0.0, 60.0),
                }
            )
    );
    addPlane(rc,
            createPlane(V3(0.0, -6.0, 0.0),
                        V3(0.0, 1.0, 0.0),
                (material) {
                    V3(0.25, 0.25, 0.25),
                    V3(0.75, 1.0, 1.0),
                    V4(0.0, 0.0, 0.0, 0.0),
                },
                (material) {
                    V3(0.75, 0.75, 0.75),
                    V3(0.24, 0.35, 0.35),
                    V4(0.0, 0.0, 0.0, 0.0f),
                }
            )
    );
   for (int i = 0; i < 3000; i++) {
        addSphere(rc,
                 createSphere(V3(frand() * 1000. - 500., frand() * 20 + 9.4,  frand() * 1000 - 200), frand() * 3 + 1,
                (material) {
                    V3(0.8, 0.8, 0.8),
                    V3(0.4, 0.4, 0.4),
                    V4(frand() * 0.3, frand() * 0.3, frand() * 0.3, frand() * 60),
                })
        );
    }
    //Allocate the output frame buffer
    char* output_buffer = aligned_malloc(64, sizeof(char[3]) * w * h);
    render_parameters params = (render_parameters){
        .x = w,
        .y = h,
        .num_threads = num_threads,
        .max_bounces = max_bounces,
        .rays_per_pixel = rays_per_pixel,
        .origin_x = 0.0,
        .origin_y = 0.0,
        .output_buffer = output_buffer
        //origin_z and frame filled in loop

    };
    for (int i = 0; i < num_frames; i++) {
        params.origin_z = -10 + (i/10.0);
    	fprintf(stderr, "Generating frame %d\n", i);
    	renderScene(rc, &params);
        char filename[256];
        sprintf(filename, "render_output/%d.bmp", i);
        printf("Dumping frame to %s...\n", filename);
        generateBmp(filename, output_buffer, w, h);
    }
    return 0;
}
