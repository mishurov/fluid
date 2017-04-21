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


void main() {
    float T = texture2D(temperature, uv).x;
    vec2 V = texture2D(velocity, uv).xy;

    gl_FragColor = vec4(V.x, V.y, 1.0, 1.0);

    if (T > ambient_temperature) {
        float D = texture2D(density, uv).x;
        V += (dt * px1 * (T - ambient_temperature) * sigma - D * kappa ) * gravity;
        gl_FragColor = vec4(V.x, V.y, 1.0, 1.0);
    }
}
