precision highp float;

uniform sampler2D sampler;
uniform vec3 bg_color;
uniform vec3 fg_color;
varying vec2 uv;


void main() {
    float l = texture2D(sampler, uv).r;
    vec3 fill_color = mix(bg_color, fg_color, l);
    gl_FragColor = vec4(fill_color, 1.0);
}

