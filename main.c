#include <stdlib.h>
#include <stdio.h>
#include "trace.h"

float frand() {
    return rand() / (float)RAND_MAX;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage:\n\traytracer <width> <height> <num_threads>\n");
        return 255;
    }
    int num_threads = atoi(argv[3]);
    int w = atoi(argv[1]);
    int h = atoi(argv[2]);
    render_context* rc = createRenderContext();
    addEntity(rc,
            createSphere(VEC3F(2000.0, 10000.0, -20000.0), 100.0f,
                (material) {
                    VEC3F(0.0, 0.0, 0.0),
                    VEC3F(0.0, 0.0, 0.0),
                    VEC3F(1.0f, 1.0f, 1.0f),
                    200000000.0
                }
            )
    );
    addEntity(rc,
            createSphere(VEC3F(0.0, 4.0, 53.0), 2.3,
                (material) {
                    VEC3F(1.0, 1.0, 1.0),
                    VEC3F(0.2, 0.2, 0.2),
                    VEC3F(0.05f, 0.05f, 0.05f),
                    0.0
                }
            )
    );
    addEntity(rc,
            createSphere(VEC3F(0.0, 0.0, 55.2), 1.5,
                (material) {
                    VEC3F(0.6, 0.6, 0.6),
                    VEC3F(0.0, 0.0, 0.0),
                    VEC3F(0.0, 0.0, 1.0),
                    60.0
                }
            )
    );
    addEntity(rc,
            createSphere(VEC3F(-3.0, 0.0, 50.0), 1.5,
                (material) {
                    VEC3F(0.6, 0.6, 0.6),
                    VEC3F(0.0, 0.0, 0.0),
                    VEC3F(0.0, 1.0, 0.0),
                    60.0,
                }
            )
    );
    addEntity(rc,
            createSphere(VEC3F(3.0, 0.0, 50.0), 1.5,
                (material) {
                    VEC3F(0.6, 0.6, 0.6),
                    VEC3F(0.0, 0.0, 0.0),
                    VEC3F(1.0, 0.0, 0.0),
                    60.0,
                }
            )
    );
    addEntity(rc,
            createPlane(VEC3F(0.0, -6.0, 0.0),
                        VEC3F(0.0, 1.0, 0.0),
                (material) {
                    VEC3F(0.5, 0.5, 0.5),
                    VEC3F(0.75, 1.0, 1.0),
                    VEC3F(0.0, 0.0, 0.0),
                    0.0,
                },
                (material) {
                    VEC3F(0.75, 0.75, 0.75),
                    VEC3F(0.24, 0.35, 0.35),
                    VEC3F(0.0, 0.0, 0.0),
                    0.0,
                }
            )
    );
    for (int i = 0; i < 200; i++) {
        addEntity(rc,
                 createSphere(VEC3F((i % 10) * 10 -50, frand() * 10 + 9.4,  (i / 10) * 10 + 40 ), frand() * 2.0 + 1.0,
                (material) {
                    VEC3F(0.8, 0.8, 0.8),
                    VEC3F(0.4, 0.4, 0.4),
                    VEC3F(frand() * 0.3, frand() * 0.3, frand() * 0.3),
                    3.0
                })
        );
    }
    renderScene(rc, w, h, num_threads);
    return 0;
}
