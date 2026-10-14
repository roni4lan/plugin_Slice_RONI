#version 330
in vec2 uv;
out vec4 fragColor;
uniform float perimeter, softness, opacity, coreWidth, gradientOffset, gradientRepeat;
uniform float hueShift, saturation, brightness;
uniform int colorMode, profile, stopCount;
uniform vec4 colorA, colorB, coreColor, stopColors[8];
uniform float stopPositions[8];
vec3 hsv(vec3 c){
    vec4 K=vec4(0.,-1./3.,2./3.,-1.);
    vec4 p=mix(vec4(c.bg,K.wz),vec4(c.gb,K.xy),step(c.b,c.g));
    vec4 q=mix(vec4(p.xyw,c.r),vec4(c.r,p.yzx),step(p.x,c.r));
    float d=q.x-min(q.w,q.y),e=1.e-10;
    return vec3(abs(q.z+(q.w-q.y)/(6.*d+e)),d/(q.x+e),q.x);
}
vec3 rgb(vec3 c){vec3 p=abs(fract(c.xxx+vec3(0.,2./3.,1./3.))*6.-3.);return c.z*mix(vec3(1.),clamp(p-1.,0.,1.),c.y);}
void main(){
    float t=fract(uv.x/max(perimeter,0.001)*gradientRepeat+gradientOffset);
    vec4 c=colorA;
    if(colorMode==1){
        c=mix(colorA,colorB,t);
        if(stopCount>0){c=stopColors[0];for(int i=1;i<stopCount;++i){float blend=clamp((t-stopPositions[i-1])/max(0.0001,stopPositions[i]-stopPositions[i-1]),0.,1.);c=mix(c,stopColors[i],blend);}}
    }
    float v=abs(uv.y),aa=max(fwidth(v),0.001);
    float alpha=1.-smoothstep(1.-aa,1.,v);
    if(profile==1) alpha*=1.-smoothstep(1.-max(softness,0.001),1.,v);
    if(profile==2) alpha*=exp(-v*v/mix(0.02,0.5,softness));
    if(coreWidth>0.)c=mix(c,coreColor,1.-smoothstep(coreWidth-aa,coreWidth+aa,v));
    vec3 h=hsv(c.rgb);h.x+=hueShift;h.y=clamp(h.y*saturation,0.,1.);h.z*=brightness;
    float a=clamp(alpha*c.a*opacity,0.,1.);
    fragColor=vec4(rgb(h)*a,a);
}
