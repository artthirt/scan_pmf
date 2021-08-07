#version 400 core
in vec4 aPos;
in vec2 aTex;
uniform mat4 mvp;
out vec2 vTex;
void main(){
    vTex = aTex;
    gl_Position = mvp * aPos;
}
