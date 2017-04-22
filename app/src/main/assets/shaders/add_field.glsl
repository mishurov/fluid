precision highp float;

uniform vec2 force;
uniform vec2 scale;
uniform vec2 px;
varying vec2 uv;
uniform sampler2D source;
uniform vec2 center;

#include "converse.glsl"

void main() {
    vec4 s_c = texture2D(source, uv);
    vec2 s = unpack(s_c);

    s += epsilon*100.;
    float dist = 1.0-min(length((uv - center) / scale), 1.0);
    s += force * dist;
    gl_FragColor = pack(s);
}
