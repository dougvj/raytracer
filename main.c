#include <stdlib.h>
#include <stdio.h>
#include "trace.h"
#include <time.h>
#include "geometric_primitives.h"

float frand() {
    return rand() / (float)RAND_MAX;
}

int main(int argc, char** argv) {
    if (argc < 6) {
        fprintf(stderr, "Usage:\n\traytracer <width> <height> <num_threads> <max_bounces> <rays_per_pixel\n");
        return 1;
    }
    srand(100);
    //srand(time(NULL));
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
    render_parameters params = (render_parameters){
        .x = w,
        .y = h,
        .num_threads = num_threads,
        .max_bounces = max_bounces,
        .rays_per_pixel = rays_per_pixel,
        .origin_x = 0.0,
        .origin_y = 0.0,
        //origin_z and frame filled in loop

    };
    for (int i = 0; i < 1; i++) {
        params.origin_z = -10 + (i/10.0);
        params.frame = i;
    	fprintf(stderr, "Generating frame %d\n", i);
    	renderScene(rc, &params);
    }
    return 0;
}
