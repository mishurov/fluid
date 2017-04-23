precision mediump float;

uniform sampler2D pressure;
uniform sampler2D velocity;
uniform float scale;
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

    vec4 v_c = texture2D(velocity, uv);
    vec2 v = unpack2FloatsTo4bytes(v_c);

    vec2 res = (v - (vec2(x1, y1) - vec2(x0, y0)) * 0.5) * scale;
    gl_FragColor = pack2FloatsTo4bytes(res);
}
