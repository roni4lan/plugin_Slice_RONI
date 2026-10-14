#version 330
uniform sampler2D sourceTex, destinationTex;
uniform int blendMode;
in vec2 uv;
out vec4 fragColor;
void main(){
    vec4 s=texture(sourceTex,uv),d=texture(destinationTex,uv);
    vec3 cs=s.a>0.?s.rgb/s.a:vec3(0.),cd=d.a>0.?d.rgb/d.a:vec3(0.);
    vec3 blended=blendMode==3?cs*cd:cs+cd-cs*cd;
    fragColor=vec4(s.rgb*(1.-d.a)+d.rgb*(1.-s.a)+blended*s.a*d.a,s.a+d.a-s.a*d.a);
}
