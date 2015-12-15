#include <stdlib.h>
#include <stdio.h>
#include "trace.h"

int main(int argc, char** argv) {
    render_context* rc = createRenderContext();
    addEntity(rc,
            createSphere(FLOAT4_3f(0.0f, -0.3f, 15.0f), 1.5f,
                (material) {
                    FLOAT4_3f(1.0f, 1.0f, 1.0f),
                    FLOAT4_3f(0.0f, 0.0f, 0.0f),
                    FLOAT4_3f(0.1f, 0.1f, 0.1f)
                }
            )
    );
    addEntity(rc,
            createSphere(FLOAT4_3f(-3.0f, -3.0f, 10.0f), 1.5f,
                (material) {
                    FLOAT4_3f(0.6f, 0.6f, 0.6f),
                    FLOAT4_3f(0.0f, 0.0f, 0.0f),
                    FLOAT4_3f(0.0f, 1.0f, 0.0f)
                }
            )
    );
    addEntity(rc,
            createSphere(FLOAT4_3f(3.0f, 3.0f, 10.0f), 1.5f,
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
                    FLOAT4_3f(0.0f, 0.0f, 0.0f),
                    FLOAT4_3f(0.2f, 0.2f, 0.2f)
                }
            )
    );
    renderScene(rc, 3840, 2160, 24);
    return 0;
}
