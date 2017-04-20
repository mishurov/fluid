precision highp float;

uniform sampler2D source;
uniform sampler2D velocity;
uniform float dt;
uniform float dissipation;
uniform vec2 px1;
varying vec2 uv;


void main() {
    vec2 vel = texture2D(velocity, uv).xy;
    // back in the future coordinates
    vec2 coords = uv - vel * dt * px1;
    gl_FragColor = texture2D(source, coords) * dissipation;
}
