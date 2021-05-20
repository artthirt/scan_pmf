#include "parserpmf.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QPainter>

const int MaxMedian = 15;

ParserPmf::ParserPmf()
{

}

void ParserPmf::setAngleRange(float a1, float a2)
{
    mAngleRange[0] = a1;
    mAngleRange[1] = a2;
}

matrixus_t ParserPmf::filterImage(const matrixus_t &mat)
{
    matrixus_t res;
    int h = mat.size();
    int w = mat[0].size();

    res.resize(h);
    for(size_t i = 0; i < h; ++i){
        res[i].resize(w, 0);
    }

    for(int i = 1; i < h - 1; ++i){
        for(int j = 1; j < w - 1; ++j){
            float t = mat[i - 1][j];
            float l = mat[i][j - 1];
            float r = mat[i][j + 1];
            float b = mat[i - 1][j];
            float c = mat[i][j];

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
    int szM = mMedianSize * mMedianSize;
    int off = mMedianSize / 2;
    int beg = -off;
    int end = beg + mMedianSize;
#pragma omp parallel for
    for(int i = off; i < h - off; ++i){
        for(int j = off; j < w - off; ++j){
            float srt[MaxMedian * MaxMedian];
            int it = 0;
            for(int k = beg; k < end; ++k){
                for(int l = beg; l < end; ++l){
                    srt[it++] = mat[i + k][j + l];
                }
            }
            std::sort(&srt[0], &srt[szM]);
            res[i][j] = srt[szM/2];
        }
    }
    return res;
}

void ParserPmf::applyInv(matrixus_t &mat, int max)
{
    for(int i = 0; i < mat.size(); ++i){
        for(int j = 0; j < mat[i].size(); ++j){
            mat[i][j] = max - mat[i][j];
        }
    }
}

void ParserPmf::saveToImage(const QString &fn, const matrixus_t &mat, int max,
                            bool useMask, const QRect &rect)
{
    int h = mat.size();
    int w = mat[0].size();

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

    if(mUseFilter){
        for(int i = 0; i < mBlurIter; ++i)
            filt = filterImage(filt);
    }

    QImage im(w, h, QImage::Format_Grayscale16);

    for(int i = 0; i < h; ++i){
        ushort *d = (ushort*)im.scanLine(i);
        for(int j = 0; j < w; ++j){
            d[j] = std::max(0.f, std::min(65535.f, 1.f * filt[i][j]/mMax * 65535.f));
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
    mData.reserve(1000 * 1000);

    QFile f(pmf);
    if(!f.open(QIODevice::ReadOnly))
        return matrixus_t();

    QString s = f.readAll();
    mNumlist1 = s.splitRef("\n");

    max = -99999999;
    for(auto a: mNumlist1){
        mNumlist2 = a.split(" ");
        mDataLine.clear();
        for(auto b: mNumlist2){
            if(!b.trimmed().isEmpty()){
                int num = b.toInt();
                max = qMax(max, num);
                mDataLine.push_back(num);
            }
        }
        if(!mDataLine.empty())
            mData.push_back(mDataLine);
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

    qDebug("size of data [%dx%d]; max %d; angle %f", mData.size(), mData[0].size(), max, angle);

    if(save)
        saveToImage(newfn, mData, max, mUseMask, rect);

    f.close();

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

void ParserPmf::apply(matrixus_t &im)
{
    if(mMask.size() != im.size() && mMask[0].size() != im[0].size())
        return;

    for(int i = 0; i < mMask.size(); ++i){
        for(int j = 0; j < mMask[i].size(); ++j){
            im[i][j] = std::max(0.f, mMask[i][j] - im[i][j]);
            if(im[i][j] < mThreshold)
                im[i][j] = 0;
//            if(im[i][j] >= mMask[i][j]){
//                im[i][j] = 0;
//            }
        }
    }
}

void ParserPmf::loadMask(const QString &mask, const QRect &rect)
{
    mNeededWidth = -1;
    mUseFilter = false;
    mUseInv = false;
    int max;
    mMask = scanFile(mask, "", max, QRect());

    if(rect.isNull() || rect.width() > mMask[0].size() || rect.height() > mMask.size()){
        return;
    }

    int x = rect.x();
    int y = rect.y();
    int w = rect.width();
    int h = rect.height();

    matrixus_t msk;
    msk.resize(h);
    for(auto &it: msk){
        it.resize(w);
    }
    for(int j = y, k = 0; j < y + h; ++j, ++k){
        for(int i = x, l = 0; i < x + w; ++i, ++l){
            msk[k][l] = mMask[j][i];
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

void ParserPmf::setBlurIter(int val)
{
    mBlurIter = val;
}

void ParserPmf::setMedianSize(int val)
{
    mMedianSize = val;
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
        for(int j = 0; j < m[i].size(); ++j){
            m[i][j] = 0;
            m[m.size() - i - 1][j] = 0;
        }
    }
    for(int i = 0; i < m.size(); i++) {
        for(int j = 0; j < m[i].size(); j += 256){
            m[i][j + 0] = 0;
            if(j > 0 && j < m[i].size() - 1){
                m[i][j - 1] = (m[i][j - 2] + m[i][j + 2])/2;
                m[i][j] = (m[i][j - 1] + m[i][j + 2])/2;
            }
//            if(j > 0 && j < m[i].size() - 1){
//                m[i][j - 1] = 0;
//            }
//            if(j < m[i].size() - 1)
//                m[i][j + 1] = 0;
        }
    }
}
