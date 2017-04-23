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
uniform vec2 px;

#include "converse.glsl"

void main() {
    vec4 t_c = texture2D(temperature, uv);
    float t = unpack1(t_c);
 
    vec4 v_c = texture2D(velocity, uv);
    vec2 v = unpack2(v_c);

    gl_FragColor = pack2(v);

    if (t > ambient_temperature) {
        vec4 d_c = texture2D(density, uv);
        float d = unpack1(d_c);
        v += (dt * px1 * (t - ambient_temperature) * sigma - d * kappa) * gravity;
        gl_FragColor = pack2(v);
    }
}
