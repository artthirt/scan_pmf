#ifndef TVDENOISER_H
#define TVDENOISER_H

#include "matutils.h"

class TVDenoiser
{
public:
    TVDenoiser();

    void setMaximum(float val){
        mMaximum = val;
    }

    void denoise(const matrixus_t& In, matrixus_t& Out, float lambda = 1, int niter = 100);

private:
    float mMaximum = 255;
};

#endif // TVDENOISER_H
