//#version 400 core
attribute vec4 aPos;
attribute vec2 aTex;
uniform mat4 mvp;
varying vec2 vTex;
void main(){
    vTex = aTex;
    gl_Position = mvp * aPos;
}
