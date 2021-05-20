#ifndef PARSERPMF_H
#define PARSERPMF_H

#include <QVector>
#include <QRect>
#include <cstdint>

typedef std::vector< std::vector< float > > matrixus_t;
typedef std::vector< float > vectorus_t;

class ParserPmf{
public:
    ParserPmf();

    void setAngleRange(float a1, float a2);

    matrixus_t filterImage(const matrixus_t& mat);

    void applyInv(matrixus_t& mat, int max);

    void saveToImage(const QString &fn, const matrixus_t& mat, int max, bool useMask,
                     const QRect& rect);

    matrixus_t scanFile(const QString& pmf, const QString& dsc, int& max, const QRect& rect, float Angle = -1, bool save = false);

    void scanDir(const QString& path, const QString& prefix);

    void apply(matrixus_t& im);

    void loadMask(const QString& mask, const QRect& rect);

    void setUseMask(bool val);

    void setUseFilter(bool val);
    void setUseInv(bool val);
    void setNeededWidth(int w);
    void setRect(const QRect& rect);
    void setMax(float val);
    void setRemove256RemoveLine(bool val);
    void setThreshold(float val);
    void setBlurIter(int val);
    void setMedianSize(int val);

    void clearOutputDir();

private:
    matrixus_t mData;
    vectorus_t mDataLine;
    QVector<QStringRef> mNumlist1;
    QVector<QStringRef> mNumlist2;
    bool mUseFilter = false;
    bool mUseInv = false;
    int mNeededWidth = -1;
    QRect mRect;
    bool mUseMask = false;
    matrixus_t mMask;
    float mMax = -1;
    QString mSaveDir = "data/";
    float mAngleRange[2] = {0, 0};
    bool mRemove256BoxLine = false;
    float mThreshold = 10;
    int mBlurIter = 5;
    int mMedianSize = 3;

    void applyRemove256(matrixus_t& m);
};


#endif // PARSERPMF_H
