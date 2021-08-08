#ifndef OUTPUTIMAGE_H
#define OUTPUTIMAGE_H

#include <QTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_0_Core>
#include <QMatrix4x4>

#include "mat.h"

#include "globj.h"

#ifndef GLOBJ_H
typedef QOpenGLFunctions_4_0_Core OpenGLFunctions;
#endif

class OutputImage: public QOpenGLWidget, public OpenGLFunctions
{
    Q_OBJECT
public:
    OutputImage(QWidget *w = nullptr);

    void setImage(const Mat& m);

    void setRed(float val)    { mRgb[0] = val;  mIsUpdate = true; }
    void setGreen(float val)  { mRgb[1] = val;  mIsUpdate = true; }
    void setBlue(float val)   { mRgb[2] = val;  mIsUpdate = true; }
    void setEV(float ev);
    void setGamma(bool check) { mGamma = check; mIsUpdate = true; }

    QString info() const { return mInfo; }
    int maxDimensionSize() const { return mMaxDimensionSize; }

    void setShowLine(bool use);
    void setYOffsetLine(float val);
    void setXOffsetLine(float val);

signals:
    void initialize();

public slots:
    void onTimeout();
    void updateImage(Mat);

private:
    QOpenGLShaderProgram mProg;
    bool mIsUpdate = false;
    bool mIsInit = false;
    bool mIsTexUpdate = false;
    QMatrix4x4 mModel;
    QMatrix4x4 mProj;
    QMatrix4x4 mMVP;

    globj mLine;
    bool mShowLine = true;
    float mYOffset = 0;
    float mXOffset = 0;

    Mat mImage;

    bool mRender = true;

    int mMvpInt = -1;
    int mVecAttrib = -1;
    int mTexAttrib = -1;
    int mTexSampler = -1;
    int mGammaInt = -1;
    int mRGBInt = -1;

    float mRgb[4] = {1, 1, 1, 1};
    float mEV = 1;

    QString mInfo;
    int mMaxDimensionSize = 10000;

    uint32_t mTexGen = 0;

    std::vector<float> mVecA;
    std::vector<float> mTexA;
    std::vector<uint32_t> mIdx;

    bool mInit = false;
    bool mUpdate = false;
    bool mTexUpdate = false;

    bool mLBDown = false;
    QPointF mMPos;
    QPointF mTranslate;
    float mScale = 1;
    int mGamma = 0;
    GLuint pbo_buffer = 0;
    int mIFmt = 0;
    int mFmt = 0;
    int mTp = 0;

    QTimer mTimer;

    Size mSizeImage;

    void drawTexture();
    void generateTexture();
    void setViewport(float w, float h);
    void updateLine();
    // QOpenGLWidget interface
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};

#endif // OUTPUTIMAGE_H
