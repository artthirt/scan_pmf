#include "outputimage.h"

#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>

#define SHOWGLERROR()  qDebug("line %d: glGetError=%d", __LINE__, glGetError())

OutputImage::OutputImage(QWidget *w)
    : QOpenGLWidget(w)
    , OpenGLFunctions()
{
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    mTimer.start(16);

    setAutoFillBackground(false);
}

void OutputImage::setImage(const Mat &m)
{
    mImage = m;
    mIsUpdate = true;
    mIsTexUpdate = true;
}

void OutputImage::setEV(float ev)
{
    mEV = ev;
    mIsUpdate = true;
}

void OutputImage::onTimeout()
{
    if(!mIsInit)
        return;
    if(mIsTexUpdate && mRender){
        mIsTexUpdate = false;
        generateTexture();
    }

    if(mIsUpdate && mRender){
        mIsUpdate = false;
        update();
    }
}

void OutputImage::updateImage(Mat image)
{
    setImage(image);
}

void OutputImage::drawTexture()
{
    mModel.setToIdentity();
    mModel.translate(0, 0, -2);
    mModel.scale(mScale, mScale, 1);
    mModel.translate(mTranslate.x(), -mTranslate.y(), 0);

    if(!mImage.empty()){
        float ar = 1. * width() / height();
        float arim = 1. * mImage.cols / mImage.rows;

        if(ar > arim){
            mModel.scale(arim, 1, 0);
        }else{
            mModel.scale(ar, ar/arim, 0);
        }
    }

    mMVP = mProj * mModel;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.1, 0.1, 0.1, 1);

    setViewport(width(), height());

    mProg.bind();

    const float *mat = mMVP.data();
    glUniformMatrix4fv(mMvpInt, 1, false, mat);

    float rgb[] = {
        mRgb[0] * mEV,
        mRgb[1] * mEV,
        mRgb[2] * mEV,
        1
    };

    glUniform4fv(mRGBInt, 1, rgb);

    glUniform1i(mGammaInt, mGamma);

    //showError(0);

    glUniform1i(mTexSampler, 1);

    if(mTexGen){
        //SHOWGLERROR();
//        glBindTexture(GL_TEXTURE_2D, mTexGen);
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_buffer);
        glBindTexture(GL_TEXTURE_2D, mTexGen);
        //glTexImage2D(GL_TEXTURE_2D, 0, mIFmt, mSizeImage.width, mSizeImage.height, 0, mFmt, mTp, 0);

        SHOWGLERROR();
    }
    glEnableVertexAttribArray(mVecAttrib);
    glVertexAttribPointer(mVecAttrib, 3, GL_FLOAT, false, 0, mVecA.data());
    //showError(1);
    glEnableVertexAttribArray(mTexAttrib);
    glVertexAttribPointer(mTexAttrib, 2, GL_FLOAT, false, 0, mTexA.data());
    //showError(2);
    glDrawElements(GL_TRIANGLES, mIdx.size(), GL_UNSIGNED_INT, mIdx.data());
    //glDrawArrays(GL_LINE_STRIP, 0, mVecA.size()/3);
    //showError(3);
    glDisableVertexAttribArray(mVecAttrib);
    glDisableVertexAttribArray(mTexAttrib);
    glDisable(GL_TEXTURE_2D);

    mProg.release();
}

void OutputImage::generateTexture()
{
    if(mImage.empty())
        return;

    if(!mTexGen){
        glGenTextures(1, &mTexGen);
    }

//    glBindTexture(GL_TEXTURE_2D, mTexGen);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    mIFmt = GL_R8;
    mFmt = GL_RED;
    mTp = GL_UNSIGNED_BYTE;

    int d = mImage.elemSize1();
    int ch = mImage.channels();

    if(mImage.depth() == CV_8U && mImage.channels() == 1){
        mIFmt = GL_LUMINANCE;
        mFmt = GL_LUMINANCE;
        mTp = GL_UNSIGNED_BYTE;
    }
    if(mImage.depth() == CV_8U && mImage.channels() == 3){
        mIFmt = GL_RGB;
        mFmt = GL_RGB;
        mTp = GL_UNSIGNED_BYTE;
    }
    if(mImage.depth() == CV_16U && mImage.channels() == 1){
        mIFmt = GL_LUMINANCE;
        mFmt = GL_LUMINANCE;
        mTp = GL_UNSIGNED_SHORT;
    }
    if(mImage.depth() == CV_16U && mImage.channels() == 3){
        mIFmt = GL_RGB16;
        mFmt = GL_RGB;
        mTp = GL_UNSIGNED_SHORT;
    }
    if(mSizeImage != mImage.size()){
        //SHOWGLERROR();
        if(pbo_buffer){
            glDeleteBuffers(1, &pbo_buffer);
            pbo_buffer = 0;
        }

        GLint bsize;
        glGenBuffers(1, &pbo_buffer);

//        QSize sz = QGuiApplication::primaryScreen()->size() * 2;
//        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_buffer);
//        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//        glBufferData(GL_PIXEL_UNPACK_BUFFER,
//                     GLsizeiptr(3) * sz.width() * sz.height(), nullptr, GL_STREAM_COPY);
//        glGetBufferParameteriv(GL_PIXEL_UNPACK_BUFFER, GL_BUFFER_SIZE, &bsize);
//        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        mSizeImage = mImage.size();
        //SHOWGLERROR();
    }

    GLint bsize;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_buffer);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBufferData(GL_PIXEL_UNPACK_BUFFER, ch * d * mImage.rows * mImage.cols, mImage.data.data(), GL_STREAM_COPY);
    glGetBufferParameteriv(GL_PIXEL_UNPACK_BUFFER, GL_BUFFER_SIZE, &bsize);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_buffer);
    glBindTexture(GL_TEXTURE_2D, mTexGen);
    glTexImage2D(GL_TEXTURE_2D, 0, mIFmt, mImage.cols, mImage.rows, 0, mFmt, mTp, 0);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    //SHOWGLERROR();

    //glTexImage2D(GL_TEXTURE_2D, 0, ifmt, mImage.cols, mImage.rows, 0, fmt, tp, mImage.data);
}

void OutputImage::setViewport(float w, float h)
{
    glViewport(0, 0, w, h);
    float ar = w/h;
    mProj.setToIdentity();
    mProj.frustum(-ar/2., ar/2., -1./2, 1./2, 1., 50.);
}

void OutputImage::initializeGL()
{
    QOpenGLWidget::initializeGL();
    OpenGLFunctions::initializeOpenGLFunctions();

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

    mProg.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/res/obj.vert");
    mProg.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/res/obj.frag");
    mProg.link();
    qDebug("Log:\n<<<\n%s\n>>>", mProg.log().toLatin1().data());

    mProg.bind();
    mVecAttrib = mProg.attributeLocation("aPos");
    mTexAttrib = mProg.attributeLocation("aTex");
    mMvpInt = mProg.uniformLocation("mvp");
    mTexSampler = mProg.uniformLocation("Tex");
    mGammaInt = mProg.uniformLocation("Gamma");

    mRGBInt = mProg.uniformLocation("uRgb");

    int w = 0;
    const GLubyte* ver = glGetString(GL_VERSION);
    const GLubyte* ven = glGetString(GL_VENDOR);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &w);
    qDebug("Maximum Texture size dimension %d", w);
    mInfo += QString("Version:            ") + (char*)ver;
    mInfo += "\n";
    mInfo += QString("Vendor:             ") + (char*)ven;
    mInfo += "\n";
    mInfo += QString("Maximum texture size dimension: %1").arg(w);
    mMaxDimensionSize = w;

    mIsInit = true;

    emit initialize();
}

void OutputImage::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);

    setViewport(w, h);
}

void OutputImage::paintGL()
{
    drawTexture();
}

void OutputImage::mousePressEvent(QMouseEvent *event)
{
    mLBDown = event->button() == Qt::LeftButton;
    mIsUpdate = true;
    mMPos = event->pos();
}

void OutputImage::mouseReleaseEvent(QMouseEvent *event)
{
    mLBDown = false;
    mIsUpdate = true;
    mMPos = event->pos();
}

void OutputImage::mouseMoveEvent(QMouseEvent *event)
{
    if(mLBDown){
        QPointF dlt = event->pos() - mMPos;
        mTranslate += dlt * 2./height() / mScale;
        mIsUpdate = true;
        mMPos = event->pos();
    }
}

void OutputImage::wheelEvent(QWheelEvent *event)
{
    float dlt = event->delta();
    mScale += dlt/100;
    if(mScale < 1){
        mScale = 1;
        mTranslate = QPointF();
    }
    mIsUpdate = true;
}
