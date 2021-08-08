#version 400 core
in vec4 aPos;
in vec4 aCol;
in vec2 aTex;
uniform mat4 mvp;
out vec2 vTex;
out vec4 vCol;
void main(){
    vTex = aTex;
    vCol = aCol;
    gl_Position = mvp * aPos;
}
