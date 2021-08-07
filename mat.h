#ifndef MAT_H
#define MAT_H

#include <vector>

struct Size{
    int width = 0;
    int height = 0;
};

enum {
    CV_8U,
    CV_8UC1,
    CV_8UC3,
    CV_16U,
    CV_16UC1,
    CV_16UC3,
    CV_32F,
    CV_32FC1,
    CV_32FC3,
};

class Mat
{
public:
    Mat();
    Mat(int rows, int cols, int type);
    Mat(int rows, int cols, int type, void *data, int step = 0);

    int rows = 0;
    int cols = 0;

    void resize();

    uint8_t *ptr(int y = 0);
    const uint8_t *ptr(int y = 0) const;

    bool empty() const {
        return data.empty();
    }

    int elemSize1() const;
    int depth() const;
    int channels() const;

    Size size() const;

    std::vector<uint8_t> data;

private:
    int mType = 0;
    int mChannels = 0;
    int mElemSize1 = 0;
    int mDepth = 0;
};

inline bool operator == (const Size& s1, const Size &s2){
    return s1.width == s2.width && s1.height == s2.height;
}

inline bool operator != (const Size& s1, const Size &s2){
    return !operator==(s1, s2);
}

#endif // MAT_H
