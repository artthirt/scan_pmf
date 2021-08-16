#ifndef PARSERPMF_H
#define PARSERPMF_H

#define _USE_MATH_DEFINES

#include <cmath>

#include <QVector>
#include <QRect>
#include <cstdint>
#include <QImage>

#include "matutils.h"


class ParserPmf{
public:
    ParserPmf();

    void setAngleRange(float a1, float a2);

    float maximum() const { return mMax; }

    matrixus_t filterImage(const matrixus_t& mat);

    void applyInv(matrixus_t& mat, int max);

    void saveToImage(const QString &fn, const matrixus_t& mat, int max, bool useMask,
                     const QRect& rect);

    matrixus_t scanFile(const QString& pmf, const QString& dsc,
                        int& max, const QRect& rect,
                        float Angle = -1, bool save = false,
                        QString* output = nullptr);

    void scanDir(const QString& path, const QString& prefix);
    void scanDirPgm(const QString& path, const QString& prefix);
    void scanDirPgm(const QStringList& files, const QString& prefix);

    void applyMask(matrixus_t& im);

    void loadMask(const QString& mask, const QRect& rect);
    matrixus_t mask() const {
        return mMask;
    }
    float MaximumMask() const{
        return mMaximumMask;
    }

    float progress() const {
        return mProgress;
    }

    void setUseMask(bool val);

    void setUseFilter(bool val);
    void setUseInv(bool val);
    void setNeededWidth(int w);
    void setRect(const QRect& rect);
    void setMax(float val);
    void setSub(float val);
    void setRemove256RemoveLine(bool val);
    void setThreshold(float val);
    void setThresholdAsDynamicRange(bool val);
    void setBlurIter(int val);
    void setKernelSize(int val);
    void setUseMedianFilter(bool val);
    void setUseNonLinearLut(bool val);
    void setUseTVDenoiser(bool val);
    void setTVDenoiserIter(int val);
    /**
     * @brief setSetNonLinearFun
     * 0 - sqr
     * 1 - sqrt
     * 2 - sin
     * @param fun
     */
    void setSetNonLinearFun(int fun);

    void setSaveDir(const QString& dir){
        mSaveDir = dir;
    }

    void clearOutputDir();

    QStringList filesOutputs() const {
        return mFilesOutput;
    }

private:
    matrixus_t mData;
    bool mUseFilter = false;
    bool mUseInv = false;
    int mNeededWidth = -1;
    QRect mRect;
    bool mUseMask = false;
    matrixus_t mMask;
    float mMax = -1;
    float mSub = 0;
    float mMaximumMask = 0;
    QString mSaveDir = "data/";
    float mAngleRange[2] = {0, 0};
    bool mRemove256BoxLine = false;
    float mThreshold = 0;
    float mThresholdAsDynamicRange = false;
    int mBlurIter = 5;
    int mKernelSize = 3;
    bool mUseMedian = true;
    bool mUseNonLinearLut = false;
    float mProgress = 0;
    int mFunction = 0;

    bool mUseTVDenoiser = false;
    int mTVDIter = 10;

    QStringList mFilesOutput;

    void applyRemove256(matrixus_t& m);

    void applyTVD(matrixus_t& m);
};


#endif // PARSERPMF_H
