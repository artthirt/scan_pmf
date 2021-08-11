#ifndef GLOBJ_H
#define GLOBJ_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QMatrix4x4>

typedef QOpenGLFunctions OpenGLFunctions;

struct vec4f{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 0;

    vec4f(){}
    vec4f(float x, float y, float z, float w){
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
};

inline vec4f operator*(const vec4f& A, float B){
    vec4f res = A;
    res.x *= B;
    res.y *= B;
    res.z *= B;
    res.w *= B;
    return res;
}

class globj
{
public:
    globj();

    void init(OpenGLFunctions *self);

    void drawGL(int type, const QMatrix4x4 &mvp, int groupCount = -1);

    void setColor(float r, float g, float b, float a);
    vec4f col(int id);
    void setColor(int id, float r, float g, float b, float a);
    void setColor(int id, const vec4f & col);

    void addPt(float x, float y, float z);
    void setPts(float *ptx, int size);

    void setColors(float *ptx, int size);

private:
    QOpenGLShaderProgram mProg;
    OpenGLFunctions* mSelf = nullptr;

    float mRgb[4] = {1, 1, 1, 1};

    int mMvpInt = -1;
    int mVecAttrib = -1;
    int mColAttrib = -1;
    int mTexAttrib = -1;
    int mTexSampler = -1;
    int mGammaInt = -1;
    int mRGBInt = -1;

    uint32_t mTexGen = 0;

    std::vector<float> mVecA;
    std::vector<float> mColA;
    std::vector<float> mTexA;
    std::vector<uint32_t> mIdx;
};

#endif // GLOBJ_H
