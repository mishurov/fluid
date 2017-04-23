precision mediump float;

uniform float vector_size;

#include "converse.glsl"

void main() {

    vec4 res;
    if (vector_size < 1.5) {
        gl_FragColor = packFloatTo4bytes(0.0);
    } else {
        gl_FragColor = pack2FloatsTo4bytes(vec2(0.0, 0.0));
    }
}

