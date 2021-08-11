//#version 400 core
attribute vec4 aPos;
attribute vec4 aCol;
attribute vec2 aTex;
uniform mat4 mvp;
varying vec2 vTex;
varying vec4 vCol;
void main(){
    vTex = aTex;
    vCol = aCol;
    gl_Position = mvp * aPos;
}
