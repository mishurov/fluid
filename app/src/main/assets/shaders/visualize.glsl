precision highp float;

uniform sampler2D sampler;
uniform vec3 fill_color;
varying vec2 uv;

void main()
{
    float l = texture2D(sampler, uv).x;
    gl_FragColor = vec4(fill_color*l, 1.0);
}

