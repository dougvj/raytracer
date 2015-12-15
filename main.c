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
            createSphere(FLOAT4_3f(0.0f, -0.3f, 20.0f), 2.3f,
                (material) {
                    FLOAT4_3f(1.0f, 1.0f, 1.0f),
                    FLOAT4_3f(0.1f, 0.1f, 0.1f),
                    FLOAT4_3f(0.05f, 0.05f, 0.05f)
                }
            )
    );
    addEntity(rc,
            createSphere(FLOAT4_3f(-3.0f, -3.0f, 15.0f), 1.5f,
                (material) {
                    FLOAT4_3f(0.6f, 0.6f, 0.6f),
                    FLOAT4_3f(0.0f, 0.0f, 0.0f),
                    FLOAT4_3f(0.0f, 0.0f, 1.0f)
                }
            )
    );
    addEntity(rc,
            createSphere(FLOAT4_3f(3.0f, 3.0f, 15.0f), 1.5f,
                (material) {
                    FLOAT4_3f(0.6f, 0.6f, 0.6f),
                    FLOAT4_3f(0.0f, 0.0f, 0.0f),
                    FLOAT4_3f(1.0f, 0.0f, 0.0f)
                }
            )
    );
    addEntity(rc,
            createPlane(FLOAT4_3f(0.0f, -5.0f, 0.0f),
                        FLOAT4_3f(0.0f, 1.0f, 0.0f),
                (material) {
                    FLOAT4_3f(0.6f, 0.6f, 0.6f),
                    FLOAT4_3f(1.0f, 1.0f, 1.0f),
                    FLOAT4_3f(0.0f, 0.0f, 0.0f)
                },
                (material) {
                    FLOAT4_3f(0.6f, 0.6f, 0.6f),
                    FLOAT4_3f(1.0f, 1.0f, 1.0f),
                    FLOAT4_3f(0.3f, 0.3f, 0.3f)
                }
            )
    );
    for (int i = 0; i < 25; i++) {
        addEntity(rc,
                 createSphere(FLOAT4_3f((i % 5) * 5 - 12.5f, 10.0f,  (i / 5) * 5 - 12.5f), frand() + 0.5f,
                (material) {
                    FLOAT4_3f(0.8f, 0.8f, 0.8f),
                    FLOAT4_3f(0.2f, 0.2f, 0.2f),
                    FLOAT4_3f(frand() * 0.3f, frand() * 0.3f, frand() * 0.3f)
                })
        );
    }
    renderScene(rc, w, h, num_threads);
    return 0;
}
