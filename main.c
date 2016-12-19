#include <stdlib.h>
#include <stdio.h>
#include "trace.h"
#include <time.h>
#include "geometric_primitives.h"

float frand() {
    return rand() / (float)RAND_MAX;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        fprintf(stderr, "Usage:\n\traytracer <width> <height> <num_threads> <max_iterations>\n");
        return 255;
    }
    srand(time(NULL));
    int max_iterations = atoi(argv[4]);
    int num_threads = atoi(argv[3]);
    int w = atoi(argv[1]);
    int h = atoi(argv[2]);
    render_context* rc = createRenderContext();
    addSphere(rc,
            createSphere(VEC3F(2000.0, 10000.0, -20000.0), 100.0f,
                (material) {
                    VEC3F(0.0, 0.0, 0.0),
                    VEC3F(0.0, 0.0, 0.0),
                    VEC4F(1.0f, 1.0f, 1.0f, 200000000.0),
                }
            )
    );
    addSphere(rc,
            createSphere(VEC3F(0.0, 4.0, 53.0), 2.3,
                (material) {
                    VEC3F(1.0, 1.0, 1.0),
                    VEC3F(0.2, 0.2, 0.2),
                    VEC4F(0.05f, 0.05f, 0.05f, 0.0f),
                }
            )
    );
    addSphere(rc,
            createSphere(VEC3F(0.0, 0.0, 55.2), 1.5,
                (material) {
                    VEC3F(0.6, 0.6, 0.6),
                    VEC3F(0.0, 0.0, 0.0),
                    VEC4F(0.0, 0.0, 1.0, 60),
                }
            )
    );
    addSphere(rc,
            createSphere(VEC3F(-3.0, 0.0, 50.0), 1.5,
                (material) {
                    VEC3F(0.6, 0.6, 0.6),
                    VEC3F(0.0, 0.0, 0.0),
                    VEC4F(0.0, 1.0, 0.0, 60),
                }
            )
    );
    addSphere(rc,
            createSphere(VEC3F(3.0, 0.0, 50.0), 1.5,
                (material) {
                    VEC3F(0.6, 0.6, 0.6),
                    VEC3F(0.0, 0.0, 0.0),
                    VEC4F(1.0, 0.0, 0.0, 60.0),
                }
            )
    );
    addPlane(rc,
            createPlane(VEC3F(0.0, -6.0, 0.0),
                        VEC3F(0.0, 1.0, 0.0),
                (material) {
                    VEC3F(0.25, 0.25, 0.25),
                    VEC3F(0.75, 1.0, 1.0),
                    VEC4F(0.0, 0.0, 0.0, 0.0),
                },
                (material) {
                    VEC3F(0.75, 0.75, 0.75),
                    VEC3F(0.24, 0.35, 0.35),
                    VEC4F(0.0, 0.0, 0.0, 0.0f),
                }
            )
    );
   for (int i = 0; i < 3000; i++) {
        addSphere(rc,
                 createSphere(VEC3F(frand() * 1000. - 500., frand() * 20 + 9.4,  frand() * 1000 - 200), frand() * 3 + 1,
                (material) {
                    VEC3F(0.8, 0.8, 0.8),
                    VEC3F(0.4, 0.4, 0.4),
                    VEC4F(frand() * 0.3, frand() * 0.3, frand() * 0.3, frand() * 60),
                })
        );
    }
    for (int i = 0; i < 480; i++) {
	fprintf(stderr, "Generating frame %d\n", i);
    	renderScene(rc, w, h, num_threads, max_iterations, 0.0, 0.0, -10 + (i/10.0), i);
    }
    return 0;
}
