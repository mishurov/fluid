precision highp float;

uniform vec2 force;
uniform vec2 center;
uniform vec2 scale;
uniform vec2 px;
varying vec2 uv;

#include "converse.glsl"

void main() {
    float dist = 1.0 - min(length((uv - center) / scale), 1.0);
    vec2 res = force * dist;
    gl_FragColor = pack(res);
}
