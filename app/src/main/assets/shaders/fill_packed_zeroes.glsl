precision highp float;

uniform float vector_size;

#include "converse.glsl"

void main() {
    if (vector_size == 1.)
        gl_FragColor = packFloatTo4bytes(0.);
    else if (vector_size == 2.)
        gl_FragColor = pack2FloatsTo4bytes(vec2(0., 0.));
}

