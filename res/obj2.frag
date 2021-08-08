in vec2 vTex;
in vec4 vCol;
uniform sampler2D Tex;
uniform int Gamma;
uniform vec4 uRgb;

void main(){
    gl_FragColor = uRgb * vCol;
}
