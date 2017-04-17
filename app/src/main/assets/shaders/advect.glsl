precision highp float;

uniform sampler2D source;
uniform sampler2D velocity;
uniform float dt;
uniform float dissipation;
uniform vec2 px1;
varying vec2 uv;

void main(){
    //vec2 u = texture(VelocityTexture, InverseSize * fragCoord).xy;
    //vec2 coord = InverseSize * (fragCoord - TimeStep * u);
    //FragColor = Dissipation * texture(SourceTexture, coord);

    gl_FragColor = texture2D(source, uv-texture2D(velocity, uv).xy*dt*px1)*dissipation;

}
