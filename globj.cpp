#include "globj.h"

globj::globj()
{

}

void globj::init(OpenGLFunctions *self)
{
    mSelf = self;

    mVecA = {
        -1, -1, 0,
        -1, 1, 0,
        1, 1, 0,
        1, -1, 0
    };
    mTexA = {
        0, 1,
        0, 0,
        1, 0,
        1, 1,
    };
    mIdx = {
        0, 1, 2,
        2, 3, 0,
    };

    mProg.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/res/obj2.vert");
    mProg.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/res/obj2.frag");
    mProg.link();
    qDebug("Log:\n<<<\n%s\n>>>", mProg.log().toLatin1().data());

    mProg.bind();
    mVecAttrib = mProg.attributeLocation("aPos");
    mTexAttrib = mProg.attributeLocation("aTex");
    mColAttrib = mProg.attributeLocation("aCol");
    mMvpInt = mProg.uniformLocation("mvp");
    mTexSampler = mProg.uniformLocation("Tex");
    mGammaInt = mProg.uniformLocation("Gamma");

    mRGBInt = mProg.uniformLocation("uRgb");
}

void globj::drawGL(int type, const QMatrix4x4 &mvp, int groupCount)
{
    mProg.bind();

    const float *mat = mvp.data();
    mSelf->glUniformMatrix4fv(mMvpInt, 1, false, mat);

    mSelf->glUniform4fv(mRGBInt, 1, mRgb);

    mSelf->glEnableVertexAttribArray(mVecAttrib);
    mSelf->glVertexAttribPointer(mVecAttrib, 3, GL_FLOAT, false, 0, mVecA.data());
    //showError(1);
    if(!mTexA.empty()){
        mSelf->glEnableVertexAttribArray(mTexAttrib);
        mSelf->glVertexAttribPointer(mTexAttrib, 2, GL_FLOAT, false, 0, mTexA.data());
    }else{
        mSelf->glDisable(GL_TEXTURE_2D);
    }
    if(!mColA.empty()){
        mSelf->glEnableVertexAttribArray(mColAttrib);
        mSelf->glVertexAttribPointer(mColAttrib, 4, GL_FLOAT, false, 0, mColA.data());
    }
    //showError(2);
    //mSelf->glDrawElements(GL_TRIANGLES, mIdx.size(), GL_UNSIGNED_INT, mIdx.data());
    if(groupCount <= 0){
        mSelf->glDrawArrays(type, 0, mVecA.size()/3);
    }else{
        int cnt = mVecA.size()/3 / groupCount;
        for(int i = 0; i < cnt; ++i){
            mSelf->glDrawArrays(type, i * groupCount, groupCount);
        }
    }
    //showError(3);
    mSelf->glDisableVertexAttribArray(mVecAttrib);
    mSelf->glDisableVertexAttribArray(mTexAttrib);
    mSelf->glDisableVertexAttribArray(mColAttrib);
    mSelf->glDisable(GL_TEXTURE_2D);

    mProg.release();


}

void globj::setColor(float r, float g, float b, float a)
{
    mRgb[0] = r;
    mRgb[1] = g;
    mRgb[2] = b;
    mRgb[3] = a;
}

vec4f globj::col(int id)
{
    vec4f res;
    res.x = mColA[id * 4 + 0];
    res.y = mColA[id * 4 + 1];
    res.z = mColA[id * 4 + 2];
    res.w = mColA[id * 4 + 3];
    return res;
}

void globj::setColor(int id, float r, float g, float b, float a)
{
    if(id < mColA.size()){
        mColA[id * 4 + 0] = r;
        mColA[id * 4 + 1] = g;
        mColA[id * 4 + 2] = b;
        mColA[id * 4 + 3] = a;
    }
}

void globj::setColor(int id, const vec4f &col)
{
    if(id < mColA.size()){
        mColA[id * 4 + 0] = col.x;
        mColA[id * 4 + 1] = col.y;
        mColA[id * 4 + 2] = col.z;
        mColA[id * 4 + 3] = col.w;
    }
}

void globj::addPt(float x, float y, float z)
{
    mVecA.push_back(x);
    mVecA.push_back(y);
    mVecA.push_back(z);
}

void globj::setPts(float *ptx, int size)
{
    mVecA.resize(size);
    memcpy(mVecA.data(), ptx, sizeof(float) * size);
}

void globj::setColors(float *ptx, int size)
{
    mColA.resize(size);
    memcpy(mColA.data(), ptx, sizeof(float) * size);
}
