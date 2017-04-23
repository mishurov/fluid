precision highp float;

uniform vec2 force;
uniform vec2 scale;
uniform vec2 px;
varying vec2 uv;
uniform sampler2D source;
uniform vec2 center;
uniform float vector_size;

#include "converse.glsl"

void main() {
    vec4 s_c = texture2D(source, uv);
    float dist = 1.0-min(length((uv - center) / scale), 1.0);

    if (vector_size == 1.) {
        float s = unpackFloatTo4bytes(s_c);
        s += (force * dist).x;
        gl_FragColor = packFloatTo4bytes(s);
    } else if (vector_size == 2.) {
        vec2 s = unpack2FloatsTo4bytes(s_c);
        s += force * dist;
        gl_FragColor = pack2FloatsTo4bytes(s);
    }
}
