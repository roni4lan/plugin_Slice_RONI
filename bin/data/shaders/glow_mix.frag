#version 330
uniform sampler2D borderTex,glowTex;
in vec2 uv;
out vec4 fragColor;
void main(){vec4 base=texture(borderTex,uv),g=texture(glowTex,uv);float a=clamp(g.a,0.,1.);vec3 color=g.a>0.?g.rgb/g.a:vec3(0.);fragColor=base+vec4(color*a,a)*(1.-base.a);}
