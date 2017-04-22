precision highp float;

uniform sampler2D sampler;
uniform vec3 bg_color;
uniform vec3 fg_color;
varying vec2 uv;

#include "converse.glsl"

void main() {
    vec4 l_c = texture2D(sampler, uv);
    float l = unpack(l_c).x;

    vec3 fill_color = mix(bg_color, fg_color, l);
    gl_FragColor = vec4(fill_color, 1.0);
}

