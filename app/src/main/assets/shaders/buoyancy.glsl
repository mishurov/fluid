precision highp float;

uniform sampler2D velocity;
uniform sampler2D temperature;
uniform sampler2D density;
uniform float ambient_temperature;
uniform float dt; // TimeStep
uniform float px1;
uniform float sigma;
uniform float kappa;

varying vec2 uv;


// ivec2 T = ivec2(gl_FragCoord.xy);
// float pC = texelFetch(Pressure, T, 0).r;
// vec2 v = texture2D(velocity, uv).xy;

void main()
{
    //ivec2 TC = ivec2(gl_FragCoord.xy);
    //float T = texelFetch(Temperature, TC, 0).r;
    float T = texture2D(temperature, uv).x;

    //vec2 V = texelFetch(Velocity, TC, 0).xy;
    vec2 V = texture2D(velocity, uv).xy;

    gl_FragColor = vec4(V.x, V.y, 1.0, 1.0);

    if (T > ambient_temperature) {
        //float D = texelFetch(Density, TC, 0).x;
        float D = texture2D(density, uv).x;
        V += (px1 * dt * (T - ambient_temperature) * sigma - D * kappa ) * vec2(0, 1);
        gl_FragColor = vec4(V.x, V.y, 1.0, 1.0);
    }
}
