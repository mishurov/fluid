precision highp float;

uniform sampler2D velocity;
uniform sampler2D temperature;
uniform sampler2D density;
varying vec2 uv;
uniform vec2 px;
uniform vec2 px1;
uniform float dt;
uniform float sigma;
uniform float kappa;
uniform float ambient_temperature;
uniform vec2 gravity;

#include "converse.glsl"

void main() {
    vec4 t_c = texture2D(temperature, uv);
    float t = unpackFloatTo4bytes(t_c);

    vec4 v_c = texture2D(velocity, uv);
    vec2 v = unpack2FloatsTo4bytes(v_c);

    gl_FragColor = pack2FloatsTo4bytes(v);

    if (t > ambient_temperature) {
        vec4 d_c = texture2D(density, uv);
        float d = unpackFloatTo4bytes(d_c);
        v += (dt * px1 * (t - ambient_temperature) * sigma - d * kappa) * gravity;
        gl_FragColor = pack2FloatsTo4bytes(v);
    }
}
