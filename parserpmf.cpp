#include "parserpmf.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QPainter>

#include <fstream>

#define _USE_MATH_DEFINES

#include <cmath>

const int MaxMedian = 15;

matrixus_t readPgm(const std::string& fileName)
{
    using namespace std;

    matrixus_t out;

    fstream fs(fileName, fstream::in | fstream::binary);
    string s, s1, s2;
    int w, h, mm;
    float f;
    getline(fs, s);
    do{
        getline(fs, s1);
        if(s1[0] == '#') s2 += s1;
    }while(s1[0] == '#');
    sscanf(s1.c_str(), "%d %d", &w, &h);
    getline(fs, s1);
    sscanf(s1.c_str(), "%d", &mm);

    if(mm < 256){
        Matrix<uint8_t> m;
        m.resize(h, w);
        char ch;
        int k = 0;

        //cout << s2 << endl;

        uint8_t *F = reinterpret_cast<uint8_t*>(m.ptr());
        ch = fs.get();
        while(!fs.eof()){
            uint8_t rgb;
            fs.read((char*)&rgb, sizeof(rgb));
            F[k++] = rgb;
        }
        m.convertTo(out);
    }else{
        Matrix<uint16_t> m;
        m.resize(h, w);
        char ch;
        int k = 0;

        //cout << s2 << endl;

        uint16_t *F = reinterpret_cast<uint16_t*>(m.ptr());
        ch = fs.get();
        while(!fs.eof()){
            uint16_t rgb;
            fs.read((char*)&rgb, sizeof(rgb));
            F[k++] = rgb;
        }
        m.convertTo(out);
    }

    //fs.read((char*)m.data, w * h * 3 * sizeof(float));
    fs.close();

    return out;
}

ParserPmf::ParserPmf()
{

}

void ParserPmf::setAngleRange(float a1, float a2)
{
    mAngleRange[0] = a1;
    mAngleRange[1] = a2;
}


std::vector<float> getGaussLernel(int K, float S = 2)
{
    const float pi = 4 * std::atan(1);

    std::vector<float> res;
    res.resize(K * K);

    float S2 = 2 * S * S;
    for(int i = -K/2; i <= K/2; ++i){
        for(int j = -K/2; j <= K/2; ++j){
            int off = (K/2 + i) * K + (K/2 + j);
            float s = expf(-(i * i + j * j) / S2);
            res[off] = s;
        }
    }
    float SS = 0;
    for(auto f: res){
        SS += f;
    }
    for(auto& f: res){
        f /= SS;
    }

    return res;
}

matrixus_t ParserPmf::filterImage(const matrixus_t &mat)
{
    matrixus_t res;
    int h = mat.rows;
    int w = mat.cols;

    res.resize(h, w);

    for(int i = 1; i < h - 1; ++i){
        for(int j = 1; j < w - 1; ++j){
            float t = mat.at(i - 1, j);
            float l = mat.at(i, j - 1);
            float r = mat.at(i, j + 1);
            float b = mat.at(i - 1, j);
            float c = mat.at(i, j);

            float mean4 = (t + l + r + b) / 4;
            if(c > mean4 * 1.5)
                c = t;

            if(t == l && t == r && t == b){
                c = t;
            }
            if(t == l && t == r){
                c = t;
            }
            if(l == t && l == b){
                c = l;
            }
            if(r == t && r == b){
                c = r;
            }
            if(b == l && b == r){
                c = b;
            }
        }
    }
    int szM = mKernelSize * mKernelSize;
    int off = mKernelSize / 2;
    int beg = -off;
    int end = beg + mKernelSize;

    if(mUseMedian){
#pragma omp parallel for
        for(int i = off; i < h - off; ++i){
            for(int j = off; j < w - off; ++j){
                float srt[MaxMedian * MaxMedian];
                int it = 0;
                for(int k = beg; k < end; ++k){
                    for(int l = beg; l < end; ++l){
                        srt[it++] = mat.at(i + k, j + l);
                    }
                }
                std::sort(&srt[0], &srt[szM]);
                res.at(i, j) = srt[szM/2];
            }
        }
    }else{

        std::vector<float> gaussKernel = getGaussLernel(mKernelSize);

//#pragma omp parallel for
        for(int i = off; i < h - off; ++i){
            for(int j = off; j < w - off; ++j){
                float Sum = 0;
                int cnt = 0;
                for(int k = beg; k < end; ++k){
                    for(int l = beg; l < end; ++l){
                        int id = (k + mKernelSize/2) * mKernelSize + (l + mKernelSize/2);
                        Sum += gaussKernel[id] *  mat.at(i + k, j + l);
                        cnt++;
                    }
                }
                res.at(i, j) = Sum;
            }
        }
    }
    return res;
}

void ParserPmf::applyInv(matrixus_t &mat, int max)
{
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            mat.at(i, j) = max - mat.at(i, j);
        }
    }
}

void ParserPmf::saveToImage(const QString &fn, const matrixus_t &mat, int max,
                            bool useMask, const QRect &rect)
{
    int h = mat.rows;
    int w = mat.cols;

    if(mMax < 0)
        mMax = max;

    matrixus_t filt = mat;

    if(mUseInv){
        applyInv(filt, max);
    }

    if(useMask){
        apply(filt);
    }

    if(mRemove256BoxLine){
        applyRemove256(filt);
    }

    if(mThreshold > 0){
        float fMin = mMax * mThreshold;
        if(mThresholdAsDynamicRange){
            filt.dynamicRange(fMin, mMax, 0, mMax);
        }else{
            filt.threshold(fMin, 0);
        }
    }

    if(mUseFilter){
        for(int i = 0; i < mBlurIter; ++i)
            filt = filterImage(filt);
    }

    QImage im(w, h, QImage::Format_Grayscale16);

    for(int i = 0; i < h; ++i){
        ushort *d = (ushort*)im.scanLine(i);
        for(int j = 0; j < w; ++j){
            float val = filt.at(i, j);
            d[j] = std::max(0.f, std::min(65535.f, 1.f * val * mMax));
        }
    }

    if(!rect.isEmpty()){
        im = im.copy(rect);
    }

    if(mNeededWidth > 0){
        int w = im.width();
        int h = im.height();
        if(w > h){
            float ar = 1. * w / h;
            im = im.scaled(mNeededWidth, mNeededWidth / ar);
        }
        if(h > w){
            float ar = 1. * w / h;
            im = im.scaled(mNeededWidth * ar, mNeededWidth);
        }
        QImage out(mNeededWidth, mNeededWidth, QImage::Format_Grayscale16);
        QPainter painter(&out);
        painter.fillRect(out.rect(), Qt::black);
        painter.drawImage(0, mNeededWidth/2 - im.height()/2, im);
        out.save(fn);
        return;
    }

    im.save(fn);
}

matrixus_t ParserPmf::scanFile(const QString &pmf, const QString &dsc, int &max,
                               const QRect &rect, float Angle, bool save)
{
    mData.clear();

    if(pmf.endsWith(".pgm")){
        mData = readPgm(pmf.toStdString());
        max = 255;
    }else{
        QFile f(pmf);
        if(!f.open(QIODevice::ReadOnly))
            return matrixus_t();

        QVector<QStringRef> mNumlist1;
        QVector<QStringRef> mNumlist2;

        QString s = f.readAll();
        mNumlist1 = s.splitRef("\n");

        max = -99999999;
        int h = mNumlist1.size();
        int w = 0, y = 0;
        for(auto a: mNumlist1){
            mNumlist2 = a.split(" ");
            if(!w){
                w = mNumlist2.size();
                mData.resize(h, w);
            }
            int x = 0;
            for(auto b: mNumlist2){
                if(!b.trimmed().isEmpty()){
                    int num = b.toInt();
                    max = qMax(max, num);
                    mData.at(y, x) = num;
                }
                x++;
            }
            y++;
        }
        f.close();
    }
    QFileInfo fi(pmf);
    QString newfn = mSaveDir + "/image_" + fi.baseName() + ".tif";

    float angle = 0;
    QStringList sl = fi.baseName().split("_");
    if(sl.empty()){
        angle = (sl.size() > 2)? sl[2].toFloat() : 0;
    }else{
        angle = fi.baseName().toFloat();
    }

    if(Angle >= 0){
        angle = Angle;
        newfn = mSaveDir + "/image_" + fi.baseName() + QString("_%1").arg(angle, 8, 'f', 2, QLatin1Char('0')) + ".tif";
    }

    qDebug("size of data [%dx%d]; max %d; angle %f", mData.rows, mData.cols, max, angle);

    if(save)
        saveToImage(newfn, mData, max, mUseMask, rect);


    return mData;
}

void ParserPmf::scanDir(const QString &path, const QString &prefix)
{
    QDir dir(path);

    QStringList pmf, dsc;

    for(int i = 0; i < dir.count(); ++i){
        QString fn = dir[i];
        QFileInfo fi(fn);
        if(!fn.contains(prefix)){
            continue;
        }
        if(fi.suffix() == "txt"){
            pmf << fn;
        }
        if(fi.suffix() == "dsc"){
            dsc << fn;
        }
    }
    if(pmf.size() != dsc.size()){
        qDebug("count of files dont equal");
        //return;
    }
    if(dsc.size() == pmf.size()){
        for(int i = 0; i < pmf.size(); ++i){
            float t = 1. * i / (pmf.size());
            float angle = -1;
            if(mAngleRange[1] > 0){
                angle = mAngleRange[0] + t * (mAngleRange[1] - mAngleRange[0]);
            }
            int max;
            scanFile(path + "/" + pmf[i], path + "/" + dsc[i], max, mRect, angle, true);
            //scanFiles(path + "/" + pmf[i], path + "/" + dsc[i], QRect(), true, true);
        }
    }else{
        for(int i = 0; i < pmf.size(); ++i){
            float t = 1. * i / (pmf.size());
            float angle = -1;
            if(mAngleRange[1] > 0){
                angle = mAngleRange[0] + t * (mAngleRange[1] - mAngleRange[0]);
            }
            int max;
            scanFile(path + "/" + pmf[i], "", max, mRect, angle, true);
            //scanFiles(path + "/" + pmf[i], path + "/" + dsc[i], QRect(), true, true);
        }
    }
}

void ParserPmf::scanDirPgm(const QString &path, const QString &prefix)
{
    QDir dir(path);

    QStringList pgm;

    for(int i = 0; i < dir.count(); ++i){
        QString fn = dir[i];
        QFileInfo fi(fn);
        if(!fn.contains(prefix)){
            continue;
        }
        if(fi.suffix() == "pgm"){
            pgm << fn;
        }
    }
    for(int i = 0; i < pgm.size(); ++i){
        float t = 1. * i / (pgm.size());
        float angle = -1;
        if(mAngleRange[1] > 0){
            angle = mAngleRange[0] + t * (mAngleRange[1] - mAngleRange[0]);
        }
        int max;
        scanFile(path + "/" + pgm[i], "", max, mRect, angle, true);
        //scanFiles(path + "/" + pmf[i], path + "/" + dsc[i], QRect(), true, true);
    }
}

void ParserPmf::apply(matrixus_t &im)
{
    if(mMask.rows != im.rows && mMask.cols != im.cols)
        return;

    float m1 = 999999, m2 = -9999999;
    for(int i = 0; i < mMask.rows; ++i){
        for(int j = 0; j < mMask.cols; ++j){
            im.at(i, j) = im.at(i, j) / mMask.at(i, j);
            im.at(i, j) = std::max(0.f, 1.f - im.at(i, j));
            m1 = std::min(m1, im.at(i, j));
            m2 = std::max(m2, im.at(i, j));
        }
    }
    qDebug("minmax %f %f", m1, m2);
}

void ParserPmf::loadMask(const QString &mask, const QRect &rect)
{
    mNeededWidth = -1;
    mUseFilter = false;
    mUseInv = false;
    int max;
    mMask = scanFile(mask, "", max, QRect());

    if(rect.isNull() || rect.width() > mMask.cols || rect.height() > mMask.rows){
        return;
    }

    int x = rect.x();
    int y = rect.y();
    int w = rect.width();
    int h = rect.height();

    matrixus_t msk;
    msk.resize(h, w);
    for(int j = y, k = 0; j < y + h; ++j, ++k){
        for(int i = x, l = 0; i < x + w; ++i, ++l){
            msk.at(k, l) = mMask.at(j, i);
        }
    }
    mMask = msk;
    saveToImage(mSaveDir + "/_mask.tif", mMask, max, false, QRect());
}

void ParserPmf::setUseMask(bool val)
{
    mUseMask = val;
}

void ParserPmf::setUseFilter(bool val)
{
    mUseFilter = val;
}

void ParserPmf::setUseInv(bool val)
{
    mUseInv = val;
}

void ParserPmf::setNeededWidth(int w)
{
    mNeededWidth = w;
}

void ParserPmf::setRect(const QRect &rect)
{
    mRect = rect;
}

void ParserPmf::setMax(float val)
{
    mMax = val;
}

void ParserPmf::setRemove256RemoveLine(bool val)
{
    mRemove256BoxLine = val;
}

void ParserPmf::setThreshold(float val)
{
    mThreshold = val;
}

void ParserPmf::setThresholdAsDynamicRange(bool val)
{
    mThresholdAsDynamicRange = val;
}

void ParserPmf::setBlurIter(int val)
{
    mBlurIter = val;
}

void ParserPmf::setKernelSize(int val)
{
    mKernelSize = val;
}

void ParserPmf::setUseMedianFilter(bool val)
{
    mUseMedian = val;
}

void ParserPmf::clearOutputDir()
{
    QDir dir(mSaveDir);

    for(int i = 0; i < dir.count(); ++i){
        QString fn = mSaveDir + dir[i];
        if(fn.indexOf(".tif") >= 0 && fn.indexOf("image_") >= 0){
            QFile::remove(fn);
        }
    }
}

void ParserPmf::applyRemove256(matrixus_t &m)
{
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < m.cols; ++j){
            m.at(i, j) = 0;
            m.at(m.rows - i - 1, j) = 0;
        }
    }
    for(int i = 0; i < m.rows; i++) {
        for(int j = 0; j < m.cols; j += 256){
            m.at(i, j + 0) = 0;
            if(j > 0 && j < m.cols - 1){
                m.at(i, j - 1) = (m.at(i, j - 2) + m.at(i, j + 2))/2;
                m.at(i, j) = (m.at(i, j - 1) + m.at(i, j + 2))/2;
            }
//            if(j > 0 && j < m[i].size() - 1){
//                m[i][j - 1] = 0;
//            }
//            if(j < m[i].size() - 1)
//                m[i][j + 1] = 0;
        }
    }
}
