#version 330
uniform mat4 modelViewProjectionMatrix;
in vec4 position;
in vec2 texcoord;
out vec2 uv;
void main(){ uv=texcoord; gl_Position=modelViewProjectionMatrix*position; }
