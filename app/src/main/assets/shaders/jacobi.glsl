precision highp float;

uniform sampler2D pressure;
uniform sampler2D divergence;
uniform float alpha;
uniform float beta;
uniform vec2 px;
varying vec2 uv;

void main(){

    //ivec2 T = ivec2(gl_FragCoord.xy);

    // Find neighboring pressure:
    //vec4 pN = texelFetchOffset(Pressure, T, 0, ivec2(0, 1));
    //vec4 pS = texelFetchOffset(Pressure, T, 0, ivec2(0, -1));
    //vec4 pE = texelFetchOffset(Pressure, T, 0, ivec2(1, 0));
    //vec4 pW = texelFetchOffset(Pressure, T, 0, ivec2(-1, 0));
    //vec4 pC = texelFetch(Pressure, T, 0);

    // Find neighboring obstacles:
    //vec3 oN = texelFetchOffset(Obstacles, T, 0, ivec2(0, 1)).xyz;
    //vec3 oS = texelFetchOffset(Obstacles, T, 0, ivec2(0, -1)).xyz;
    //vec3 oE = texelFetchOffset(Obstacles, T, 0, ivec2(1, 0)).xyz;
    //vec3 oW = texelFetchOffset(Obstacles, T, 0, ivec2(-1, 0)).xyz;

    // Use center pressure for solid cells:
    //if (oN.x > 0.0) pN = pC;
    //if (oS.x > 0.0) pS = pC;
    //if (oE.x > 0.0) pE = pC;
    //if (oW.x > 0.0) pW = pC;

    //vec4 bC = texelFetch(Divergence, T, 0);
    //gl_FragColor = (pW + pE + pS + pN + Alpha * bC) * InverseBeta;


    float x0 = texture2D(pressure, uv-vec2(px.x, 0)).r;
    float x1 = texture2D(pressure, uv+vec2(px.x, 0)).r;
    float y0 = texture2D(pressure, uv-vec2(0, px.y)).r;
    float y1 = texture2D(pressure, uv+vec2(0, px.y)).r;
    float d = texture2D(divergence, uv).r;
    float relaxed = (x0 + x1 + y0 + y1 + alpha * d) * beta;
    gl_FragColor = vec4(relaxed);
}
