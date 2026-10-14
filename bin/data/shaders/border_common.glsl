const float TAU=6.28318530718;
uniform float time, phaseOffset, perimeter, thickness;
uniform float dashLength, gapLength, dashSpeed, dashPhase, waveAmplitude, waveFrequency, waveSpeed;
uniform float tickSpacing, tickLength, speed, chaseLength, chaseTail, pulseRate, pulseDepth;
uniform float strobeRate, strobeDuty, noiseAmount, noiseScale, noiseSpeed, glitchAmount, glitchRate, jitter, flicker;
uniform int pattern, animation, direction, chaseCount, chaseEasing, pulseShape, noiseType, noiseAffects, seed;
uniform bool noiseEnabled, dashRoundCaps;
float hash1(float n){return fract(sin(n*127.1+float(seed)*17.71)*43758.5453);}
float hash2(vec2 p){return hash1(dot(p,vec2(12.9898,78.233)));}
vec2 gradient(vec2 p){float a=hash2(p)*TAU;return vec2(cos(a),sin(a));}
float perlinNoise(vec2 p){vec2 cell=floor(p),f=fract(p),w=f*f*f*(f*(f*6.-15.)+10.);return mix(mix(dot(gradient(cell),f),dot(gradient(cell+vec2(1,0)),f-vec2(1,0)),w.x),mix(dot(gradient(cell+vec2(0,1)),f-vec2(0,1)),dot(gradient(cell+vec2(1,1)),f-vec2(1,1)),w.x),w.y)*1.5;}
float valueNoise(vec2 p){vec2 c=floor(p),f=fract(p),w=f*f*(3.-2.*f);return mix(mix(hash2(c),hash2(c+vec2(1,0)),w.x),mix(hash2(c+vec2(0,1)),hash2(c+vec2(1,1)),w.x),w.y)*2.-1.;}
float simplexNoise(vec2 p){
    const float F=0.36602540378,G=0.2113248654;
    vec2 cell=floor(p+(p.x+p.y)*F),a=p-cell+(cell.x+cell.y)*G;
    vec2 stepCell=a.x>a.y?vec2(1,0):vec2(0,1),b=a-stepCell+G,c=a-1.+2.*G;
    vec3 w=max(vec3(0.5-dot(a,a),0.5-dot(b,b),0.5-dot(c,c)),0.);w*=w;w*=w;
    return 70.*dot(w,vec3(dot(gradient(cell),a),dot(gradient(cell+stepCell),b),dot(gradient(cell+1.),c)));
}
float worleyNoise(vec2 p){vec2 cell=floor(p),f=fract(p);float closest=2.;for(int y=-1;y<=1;++y)for(int x=-1;x<=1;++x){vec2 d=vec2(x,y),c=cell+d,offset=vec2(hash2(c),hash2(c+31.7));closest=min(closest,length(d+offset-f));}return clamp(closest,0.,1.)*2.-1.;}
float noiseAt(float u){vec2 p=vec2(u*noiseScale,time*noiseSpeed);if(noiseType==1)return simplexNoise(p);if(noiseType==2)return valueNoise(p);if(noiseType==3)return worleyNoise(p);return perlinNoise(p);}
float pulse(float t){t=fract(t);if(pulseShape==1)return 1.-abs(t*2.-1.);if(pulseShape==2)return step(0.5,t);if(pulseShape==3)return t;if(pulseShape==4)return exp(-6.*t);return 0.5+0.5*sin(TAU*t);}
float motionPhase(){float t=time*speed+phaseOffset;if(direction==1 || direction==4)t=-t;float p=fract(t);if(direction==2)p=1.-abs(p*2.-1.);if(chaseEasing==1)p=p*p*(3.-2.*p);if(chaseEasing==2)p=1.-abs(cos(p*3.14159265359))*exp(-p*0.5);return p;}
float animationAlpha(float u){
    float cycle=time*speed+phaseOffset;
    if(animation==1)return 1.-pulseDepth+pulseDepth*pulse(cycle*pulseRate);
    if(animation==6)return 1.-pulseDepth+pulseDepth*pow(pulse(cycle*pulseRate),2.);
    if(animation==2)return step(0.5,fract(cycle*pulseRate));
    if(animation==3)return 1.-step(strobeDuty,fract(cycle*strobeRate));
    if(animation>=4 && animation<=8 && animation!=6){
        float count=animation==5?float(chaseCount):1.;
        float d=fract((motionPhase()-u)*count);
        float head=1.-smoothstep(chaseLength,chaseLength+0.003,d);
        float tail=chaseTail>0.?exp(-max(0.,d-chaseLength)/max(chaseTail,0.001)*5.):0.;
        if(animation==7)return 1.-smoothstep(chaseLength,chaseLength+0.3,d);
        return max(head,tail*(1.-step(chaseLength+chaseTail,d)));
    }
    if(animation==9)return 0.5+0.5*sin(TAU*(u*float(chaseCount)-cycle));
    if(animation==10)return clamp(0.5+0.5*noiseAt(u*perimeter),0.,1.);
    if(animation==11)return step(0.4,hash1(floor(cycle*4.)));
    return 1.;
}
