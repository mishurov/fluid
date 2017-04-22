precision highp float;

uniform sampler2D pressure;
uniform sampler2D divergence;
uniform float alpha;
uniform float beta;
uniform vec2 px;
varying vec2 uv;

#include "converse.glsl"

void main() {
    vec4 x0_c = texture2D(pressure, uv-vec2(px.x, 0));
    vec4 x1_c = texture2D(pressure, uv+vec2(px.x, 0));
    vec4 y0_c = texture2D(pressure, uv-vec2(0, px.y));
    vec4 y1_c = texture2D(pressure, uv+vec2(0, px.y));
    float x0 = unpack(x0_c).x;
    float x1 = unpack(x1_c).x;
    float y0 = unpack(y0_c).x;
    float y1 = unpack(y1_c).x;

    vec4 d_c = texture2D(divergence, uv);
    float d = unpack(d_c).x;

    float relaxed = (x0 + x1 + y0 + y1 + alpha * d) * beta;
    gl_FragColor = pack(vec2(relaxed, relaxed));
}
