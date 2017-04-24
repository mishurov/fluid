precision mediump float;

varying vec2 uv;
uniform vec2 px;
uniform float vector_size;

#include "converse.glsl"

void main() {

    vec4 res;
    if (vector_size < 1.5) {
        res = packFloatTo4bytes(0.0);
        gl_FragColor = res;
    } else {
        // temporary fix for the initial gradient substract
        res = pack2FloatsTo4bytes(vec2(-0.5, 0.0));
        gl_FragColor = res;
    }
}

