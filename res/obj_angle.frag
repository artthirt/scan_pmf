precision mediump float;
varying vec2 vTex;
uniform sampler2D Tex;
uniform int Gamma;
uniform vec4 uRgb;

void main(){
    vec4 col = texture2D(Tex, vTex);

    if(Gamma > 0){
        col.rgb = pow(abs(col.rgb), vec3(1./1.9));
    }

    gl_FragColor = uRgb * col.bgra;
}
