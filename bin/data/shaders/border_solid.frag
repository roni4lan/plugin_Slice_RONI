#version 330
#pragma include "border_common.glsl"
in vec2 uv;
out vec4 fragColor;
uniform float softness, opacity, coreWidth, gradientOffset, gradientRepeat;
uniform float hueShift, saturation, brightness;
uniform int colorMode, profile, stopCount, lineCount, glowMode;
uniform bool glowEnabled, glowUseBorderColor;
uniform float lineGap, glowRadius, glowIntensity, glowFalloff, bloomGain;
uniform vec4 glowColor;
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
    if(profile>=3){int count=max(lineCount,profile==3?2:3);float closest=1000.;for(int i=0;i<count;++i){float center=(float(i)-float(count-1)*0.5)*lineGap/(thickness*0.5);closest=min(closest,abs(uv.y-center)*float(count));}v=closest;aa=max(fwidth(v),0.001);}
    float patternAlpha=1.;
    float u=uv.x+time*dashSpeed+dashPhase,period=max(dashLength+gapLength,0.001),pos=mod(u,period);
    float longitudinal=max(fwidth(uv.x),0.5);
    if(pattern==1 || pattern==3 || pattern==7){
        if(pattern==7)pos=mod(u+abs(uv.y)*thickness,period);
        patternAlpha=(1.-smoothstep(dashLength-longitudinal,dashLength+longitudinal,pos));
        if(dashRoundCaps){float dx=max(abs(pos-dashLength*0.5)-max(0.,dashLength*0.5-thickness*0.5),0.)/(thickness*0.5);patternAlpha=1.-smoothstep(1.-aa,1.,length(vec2(dx,uv.y)));}
    }
    if(pattern==2){float dx=(mod(u,period)-dashLength*0.5)/max(0.5,dashLength*0.5);patternAlpha=1.-smoothstep(1.-aa,1.,length(vec2(dx,uv.y)));}
    if(pattern==4){float dx=abs(mod(u+tickSpacing*0.5,tickSpacing)-tickSpacing*0.5);patternAlpha=1.-smoothstep(thickness*0.15,thickness*0.15+longitudinal,dx);v=abs(uv.y)*thickness/max(tickLength,0.5);}
    float alpha=1.-smoothstep(1.-aa,1.,v);
    if(profile==1) alpha*=1.-smoothstep(1.-max(softness,0.001),1.,v);
    if(profile==2) alpha*=exp(-v*v/mix(0.02,0.5,softness));
    if(coreWidth>0.)c=mix(c,coreColor,1.-smoothstep(coreWidth-aa,coreWidth+aa,v));
    float n=noiseEnabled?noiseAt(uv.x):0.;
    vec3 h=hsv(c.rgb);h.x+=hueShift;if(noiseEnabled && (noiseAffects&8)!=0)h.x+=n*0.15;h.y=clamp(h.y*saturation,0.,1.);h.z*=brightness;
    float temporal=animationAlpha(uv.x/max(perimeter,0.001))*mix(1.,0.2+0.8*hash1(floor(time*25.)),flicker);
    if(noiseEnabled && (noiseAffects&4)!=0)temporal*=clamp(0.6+0.4*n,0.,1.);
    temporal*=1.-glitchAmount*step(0.9,hash1(floor(uv.x/50.)+floor(time*glitchRate)));
    float a=clamp(alpha*c.a*opacity*patternAlpha*temporal,0.,1.);
    vec3 premul=rgb(h)*a;
    if(glowEnabled && glowMode==0 && glowRadius>0.){
        float distance=max(0.,v-1.)*thickness*0.5;
        float halo=exp(-distance/max(glowRadius,0.001)*glowFalloff)*glowIntensity*opacity*patternAlpha*temporal;
        // Fade to zero at the edge of the extra geometry, avoiding a hard halo cutoff.
        halo*=1.-smoothstep(glowRadius*0.75,glowRadius,distance);
        vec4 gc=glowUseBorderColor?vec4(rgb(h),c.a):glowColor;
        float ga=clamp(halo*gc.a,0.,1.);premul+=gc.rgb*ga*(1.-a);a+=ga*(1.-a);
    }
    fragColor=vec4(premul,a)*bloomGain;
}
