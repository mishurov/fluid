precision highp float;

uniform sampler2D source;
uniform sampler2D velocity;
uniform float dt;
uniform float dissipation;
uniform vec2 px1;
uniform vec2 px;
varying vec2 uv;

#include "converse.glsl"

void main() {
    vec4 v_c = texture2D(velocity, uv);
    vec2 v = unpack2(v_c);

    // back in the future coordinates
    vec2 coords = uv - v * dt * px1;
    vec4 s_c = texture2D(source, coords);
    vec2 s = unpack2(s_c);
    vec2 res = s * dissipation;
    gl_FragColor = pack2(res);
}
