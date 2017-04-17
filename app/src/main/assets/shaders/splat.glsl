precision highp float;

uniform vec2 point;
uniform float radius;
uniform vec3 fill_color;
varying vec2 uv;

void main()
{
    float dist = distance(uv, point); 
    if (dist < radius) {
        gl_FragColor = vec4(fill_color, 1);
    } else {
        gl_FragColor = vec4(0, 0, 0, 1);
    }
}
