#version 330
uniform sampler2D frameTex;
uniform vec2 stepUV;
in vec2 uv;
out vec4 fragColor;
void main(){vec4 result=vec4(0.);float weights=0.;for(int i=-8;i<=8;++i){float w=exp(-float(i*i)/18.);result+=texture(frameTex,uv+stepUV*float(i))*w;weights+=w;}fragColor=result/weights;}
