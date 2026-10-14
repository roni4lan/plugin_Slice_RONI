#version 330
uniform mat4 modelViewProjectionMatrix;
#pragma include "border_common.glsl"
in vec4 position;
in vec2 texcoord;
in vec3 normal;
out vec2 uv;
void main(){
    uv=texcoord;vec4 p=position;float displacement=0.;
    float theta=uv.x/max(perimeter,0.001)*waveFrequency-time*waveSpeed;
    if(pattern==6)displacement+=waveAmplitude*sin(theta*TAU);
    if(pattern==5)displacement+=waveAmplitude*(1.-4.*abs(fract(theta)-0.5));
    float n=noiseEnabled?noiseAt(uv.x):0.;
    if(noiseEnabled && (noiseAffects&1)!=0)displacement+=n*noiseAmount;
    if(noiseEnabled && (noiseAffects&2)!=0)p.xy+=normal.xy*uv.y*thickness*0.2*n;
    displacement+=jitter*(hash1(uv.x+floor(time*60.))*2.-1.);
    float g=hash1(floor(uv.x/80.)+floor(time*glitchRate));
    displacement+=glitchAmount*thickness*4.*(g-0.5)*step(0.78,g);
    if(direction>=3 && animation!=0)displacement+=waveAmplitude*pulse((time*speed+phaseOffset)*pulseRate);
    if(direction==3)displacement=abs(displacement);if(direction==4)displacement=-abs(displacement);
    p.xy+=normal.xy*displacement;gl_Position=modelViewProjectionMatrix*p;
}
