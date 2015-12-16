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
            createSphere(FLOAT4_3f(2000.0, 10000.0, -20000.0), 100.0f,
                (material) {
                    FLOAT4_3f(0.0, 0.0, 0.0),
                    FLOAT4_3f(0.0, 0.0, 0.0),
                    FLOAT4_3f(1.0f, 1.0f, 1.0f),
                    200000000.0
                }
            )
    );
    addEntity(rc,
            createSphere(FLOAT4_3f(0.0, 4.0, 53.0), 2.3,
                (material) {
                    FLOAT4_3f(1.0, 1.0, 1.0),
                    FLOAT4_3f(0.2, 0.2, 0.2),
                    FLOAT4_3f(0.05f, 0.05f, 0.05f),
                    0.0
                }
            )
    );
    addEntity(rc,
            createSphere(FLOAT4_3f(0.0, 0.0, 55.2), 1.5,
                (material) {
                    FLOAT4_3f(0.6, 0.6, 0.6),
                    FLOAT4_3f(0.0, 0.0, 0.0),
                    FLOAT4_3f(0.0, 0.0, 1.0),
                    60.0
                }
            )
    );
    addEntity(rc,
            createSphere(FLOAT4_3f(-3.0, 0.0, 50.0), 1.5,
                (material) {
                    FLOAT4_3f(0.6, 0.6, 0.6),
                    FLOAT4_3f(0.0, 0.0, 0.0),
                    FLOAT4_3f(0.0, 1.0, 0.0),
                    60.0,
                }
            )
    );
    addEntity(rc,
            createSphere(FLOAT4_3f(3.0, 0.0, 50.0), 1.5,
                (material) {
                    FLOAT4_3f(0.6, 0.6, 0.6),
                    FLOAT4_3f(0.0, 0.0, 0.0),
                    FLOAT4_3f(1.0, 0.0, 0.0),
                    60.0,
                }
            )
    );
    addEntity(rc,
            createPlane(FLOAT4_3f(0.0, -6.0, 0.0),
                        FLOAT4_3f(0.0, 1.0, 0.0),
                (material) {
                    FLOAT4_3f(0.5, 0.5, 0.5),
                    FLOAT4_3f(0.75, 1.0, 1.0),
                    FLOAT4_3f(0.0, 0.0, 0.0),
                    0.0,
                },
                (material) {
                    FLOAT4_3f(0.75, 0.75, 0.75),
                    FLOAT4_3f(0.24, 0.35, 0.35),
                    FLOAT4_3f(0.0, 0.0, 0.0),
                    0.0,
                }
            )
    );
    for (int i = 0; i < 100; i++) {
        addEntity(rc,
                 createSphere(FLOAT4_3f((i % 10) * 5 -25, 8.0,  (i / 10) * 5 + 40 ), frand() * 2.0 + 1.0,
                (material) {
                    FLOAT4_3f(0.8, 0.8, 0.8),
                    FLOAT4_3f(0.4, 0.4, 0.4),
                    FLOAT4_3f(frand() * 0.3, frand() * 0.3, frand() * 0.3),
                    25.0
                })
        );
    }
    renderScene(rc, w, h, num_threads);
    return 0;
}
