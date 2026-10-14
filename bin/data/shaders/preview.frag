#version 330
uniform sampler2D frameTex;
in vec2 uv;
out vec4 fragColor;
void main(){vec4 c=texture(frameTex,uv);fragColor=vec4(c.a>0.?c.rgb/c.a:vec3(0.),c.a);}
