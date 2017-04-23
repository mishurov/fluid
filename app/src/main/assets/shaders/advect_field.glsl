precision highp float;

uniform sampler2D source;
uniform sampler2D velocity;
uniform float dt;
uniform float dissipation;
uniform vec2 px1;
varying vec2 uv;
uniform float vector_size;

#include "converse.glsl"

void main() {
    vec4 v_c = texture2D(velocity, uv);
    vec2 v = unpack2FloatsTo4bytes(v_c);

    // back in the future coordinates
    vec2 coords = uv - v * dt * px1;
    vec4 s_c = texture2D(source, coords);

    if (vector_size == 1.) {
        float s = unpackFloatTo4bytes(s_c);
        float res = s * dissipation;
        gl_FragColor = packFloatTo4bytes(res);
    } else if (vector_size == 2.) {
        vec2 s = unpack2FloatsTo4bytes(s_c);
        vec2 res = s * dissipation;
        gl_FragColor = pack2FloatsTo4bytes(res);
    }
}
