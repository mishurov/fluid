precision highp float;

uniform sampler2D velocity;
uniform sampler2D temperature;
uniform sampler2D density;
uniform float ambient_temperature;
uniform float dt;
uniform float sigma;
uniform float kappa;
uniform vec2 gravity;
varying vec2 uv;
uniform vec2 px1;

#include "converse.glsl"

void main() {
    vec4 t_c = texture2D(temperature, uv);
    float t = unpack(t_c).x;

    vec4 v_c = texture2D(velocity, uv);
    vec2 v = unpack(v_c);

    gl_FragColor = pack(v);

    if (t > ambient_temperature) {
        vec4 d_c = texture2D(density, uv);
        float d = unpack(d_c).x;

        v += (dt * px1 * (t - ambient_temperature) * sigma - d * kappa ) * gravity;
        gl_FragColor = pack(v);
    }
}
