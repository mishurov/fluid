precision highp float;

uniform sampler2D velocity;
uniform float dt;
uniform vec2 px;
varying vec2 uv;


void main(){
    //ivec2 T = ivec2(gl_FragCoord.xy);

    // Find neighboring velocities:
    //vec2 vN = texelFetchOffset(Velocity, T, 0, ivec2(0, 1)).xy;
    //vec2 vS = texelFetchOffset(Velocity, T, 0, ivec2(0, -1)).xy;
    //vec2 vE = texelFetchOffset(Velocity, T, 0, ivec2(1, 0)).xy;
    //vec2 vW = texelFetchOffset(Velocity, T, 0, ivec2(-1, 0)).xy;

    // Find neighboring obstacles:
    //vec3 oN = texelFetchOffset(Obstacles, T, 0, ivec2(0, 1)).xyz;
    //vec3 oS = texelFetchOffset(Obstacles, T, 0, ivec2(0, -1)).xyz;
    //vec3 oE = texelFetchOffset(Obstacles, T, 0, ivec2(1, 0)).xyz;
    //vec3 oW = texelFetchOffset(Obstacles, T, 0, ivec2(-1, 0)).xyz;

    // Use obstacle velocities for solid cells:
    //if (oN.x > 0.0) vN = oN.yz;
    //if (oS.x > 0.0) vS = oS.yz;
    //if (oE.x > 0.0) vE = oE.yz;
    //if (oW.x > 0.0) vW = oW.yz;

    //float ret = HalfInverseCellSize * (vE.x - vW.x + vN.y - vS.y);
    //gl_FragColor = vec4(ret, ret, ret, ret);

    // neighboring velocities
    float x0 = texture2D(velocity, uv-vec2(px.x, 0)).x;
    float x1 = texture2D(velocity, uv+vec2(px.x, 0)).x;
    float y0 = texture2D(velocity, uv-vec2(0, px.y)).y;
    float y1 = texture2D(velocity, uv+vec2(0, px.y)).y;
    float divergence = (x1 - x0 + y1 - y0) * 0.5;
    gl_FragColor = vec4(divergence);
}
