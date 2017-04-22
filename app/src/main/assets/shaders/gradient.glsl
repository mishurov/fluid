precision highp float;

uniform sampler2D pressure;
uniform sampler2D velocity;
uniform float alpha;
uniform float beta;
uniform float scale;
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

    vec4 v_c = texture2D(velocity, uv);
    vec2 v = unpack(v_c);

    vec2 res = (v - (vec2(x1, y1) - vec2(x0, y0)) * 0.5) * scale;
    gl_FragColor = pack(res);
}
