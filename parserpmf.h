#ifndef PARSERPMF_H
#define PARSERPMF_H

#include <QVector>
#include <QRect>
#include <cstdint>


template<typename T>
class Matrix{
public:
    int rows = 0;
    int cols = 0;
    std::vector<T> data;

    size_t total() const{
        return rows * cols;
    }
    bool empty() const{
        return data.empty();
    }

    void resize(int rows, int cols){
        this->rows = rows;
        this->cols = cols;
        data.resize(rows * cols, 0);
    }
    void clear(){
        this->rows = 0;
        this->cols = 0;
        data.clear();
    }
    T& at(int y, int x){
        return *(reinterpret_cast<float*>(data.data()) + y * cols + x);
    }
    T at(int y, int x)const{
        return *(reinterpret_cast<const float*>(data.data()) + y * cols + x);
    }
    float* ptr(int y = 0){
        return reinterpret_cast<float*>(data.data()) + y * cols;
    }
    float* ptr(int y = 0) const{
        return reinterpret_cast<float*>(data.data()) + y * cols;
    }
    T& operator[](int i){
        return data[i];
    }
    T operator[](int i) const{
        return data[i];
    }

    template<typename N>
    void convertTo(Matrix<N>& out) const {
        out.resize(rows, cols);
        for(size_t i = 0; i < total(); ++i){
            out[i] = data[i];
        }
    }

    void threshold(T Min, T NewVal){
        for(size_t i = 0; i < total(); ++i){
            T val = data[i];
            data[i] = val < Min? NewVal : val;
        }
    }
    void dynamicRange(T Min, T Max, T NewMin, T NewMax){
        for(size_t i = 0; i < total(); ++i){
            T val = data[i];
            val = NewMin + (val - Min) / (Max - Min) * (NewMax - NewMin);
            data[i] = std::max(NewMin, std::min(NewMax, val));
        }
    }
};

typedef Matrix<float> matrixus_t;
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
    void scanDirPgm(const QString& path, const QString& prefix);

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
    void setThresholdAsDynamicRange(bool val);
    void setBlurIter(int val);
    void setKernelSize(int val);
    void setUseMedianFilter(bool val);

    void clearOutputDir();

private:
    matrixus_t mData;
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
    float mThreshold = 0;
    float mThresholdAsDynamicRange = false;
    int mBlurIter = 5;
    int mKernelSize = 3;
    bool mUseMedian = true;

    void applyRemove256(matrixus_t& m);
};


#endif // PARSERPMF_H
