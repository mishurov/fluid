precision highp float;

uniform sampler2D pressure;
uniform sampler2D velocity;
uniform float alpha;
uniform float beta;
uniform float scale;
uniform vec2 px;
varying vec2 uv;


void main() {
    //ivec2 T = ivec2(gl_FragCoord.xy);

    // uniform sampler2D Velocity;
    // uniform sampler2D Pressure;
    // uniform sampler2D Obstacles;
    // uniform float GradientScale;
    // Find neighboring pressure:
    //float pN = texelFetchOffset(Pressure, T, 0, ivec2(0, 1)).r;
    //float pS = texelFetchOffset(Pressure, T, 0, ivec2(0, -1)).r;
    //float pE = texelFetchOffset(Pressure, T, 0, ivec2(1, 0)).r;
    //float pW = texelFetchOffset(Pressure, T, 0, ivec2(-1, 0)).r;
    //float pC = texelFetch(Pressure, T, 0).r;

    // Enforce the free-slip boundary condition:
    //vec2 oldV = texelFetch(Velocity, T, 0).xy;
    //vec2 grad = vec2(pE - pW, pN - pS) * GradientScale;
    //vec2 newV = oldV - grad;
    //vec2 ret = (vMask * newV) + obstV;  
    //gl_FragColor = vec4(ret.x, ret.y, 0.0, 0.0);  


    float x0 = texture2D(pressure, uv-vec2(px.x, 0)).x;
    float x1 = texture2D(pressure, uv+vec2(px.x, 0)).x;
    float y0 = texture2D(pressure, uv-vec2(0, px.y)).x;
    float y1 = texture2D(pressure, uv+vec2(0, px.y)).x;
    vec2 v = texture2D(velocity, uv).xy;
    gl_FragColor = vec4((v-(vec2(x1, y1)-vec2(x0, y0))*0.5)*scale, 1.0, 1.0);
}
