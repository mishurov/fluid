precision mediump float;

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
    float x0 = unpackFloatTo4bytes(x0_c);
    float x1 = unpackFloatTo4bytes(x1_c);
    float y0 = unpackFloatTo4bytes(y0_c);
    float y1 = unpackFloatTo4bytes(y1_c);

    vec4 d_c = texture2D(divergence, uv);
    float d = unpackFloatTo4bytes(d_c);

    float relaxed = (x0 + x1 + y0 + y1 + alpha * d) * beta;
    gl_FragColor = packFloatTo4bytes(relaxed);
}
