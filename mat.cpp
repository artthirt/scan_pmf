#include "mat.h"

const int ElemSizes[] = {
    1,//CV_8U,
    1,//CV_8UC1,
    1,//CV_8UC3,
    2,//CV_16U,
    2,//CV_16UC1,
    2,//CV_16UC3,
    4,//CV_32F,
    4,//CV_32FC1,
    4,//CV_32FC3,
};

const int Depths[] = {
    CV_8U,//CV_8U,
    CV_8U,//CV_8UC1,
    CV_8U,//CV_8UC3,
    CV_16U,//CV_16U,
    CV_16U,//CV_16UC1,
    CV_16U,//CV_16UC3,
    CV_32F,//CV_32F,
    CV_32F,//CV_32FC1,
    CV_32F,//CV_32FC3,
};

const int Channels[] = {
    1,//CV_8U,
    1,//CV_8UC1,
    3,//CV_8UC3,
    1,//CV_16U,
    1,//CV_16UC1,
    3,//CV_16UC3,
    1,//CV_32F,
    1,//CV_32FC1,
    3,//CV_32FC3,
};


Mat::Mat()
{

}

Mat::Mat(int rows, int cols, int type)
{
    this->rows = rows;
    this->cols = cols;
    this->mType = type;

    mElemSize1 = ElemSizes[type];
    mDepth = Depths[type];
    mChannels = Channels[type];

    resize();
}

Mat::Mat(int rows, int cols, int type, void *data, int step)
{
    this->rows = rows;
    this->cols = cols;
    this->mType = type;

    mElemSize1 = ElemSizes[type];
    mDepth = Depths[type];
    mChannels = Channels[type];

    resize();

    if(step == 0){
        memcpy(this->data.data(), data, this->data.size());
    }else{
        int pitch = cols * mChannels * mElemSize1;
        for(int i = 0; i < rows; ++i){
            uint8_t* In = reinterpret_cast<uint8_t*>(data) + i * step;
            uint8_t* Out = ptr(i);
            memcpy(Out, In, pitch);
        }
    }
}

void Mat::resize()
{
    data.resize(rows * cols * mChannels * mElemSize1);
}

uint8_t *Mat::ptr(int y)
{
    int off = cols * y * mElemSize1 * mChannels;
    return data.data() + off;
}

const uint8_t *Mat::ptr(int y) const
{
    int off = cols * y * mElemSize1 * mChannels;
    return data.data() + off;
}

int Mat::elemSize1() const
{
    return mElemSize1;
}

int Mat::depth() const
{
    return mDepth;
}

int Mat::channels() const
{
    return mChannels;
}

Size Mat::size() const
{
    return {cols, rows};
}
