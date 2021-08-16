#include "tvdenoiser.h"

matrixus_t divY(const matrixus_t &A)
{
    matrixus_t c = matrixus_t::zeros(A.rows, A.cols);

#pragma omp parallel for
    for(int y = 0; y < c.rows - 1; ++y){
        for(int x = 0; x < c.cols; ++x){
            c.at(y, x) = A.at(y + 1, x) - A.at(y, x);
        }
    }
    int y = A.rows - 1;
    for(int x = 0; x < c.cols; ++x){
        c.at(y, x) = A.at(y, x) - A.at(y, x);
    }

    return c;
}

matrixus_t divX(const matrixus_t &A)
{
    matrixus_t c = matrixus_t::zeros(A.rows, A.cols);

#pragma omp parallel for
    for(int y = 0; y < c.rows; ++y){
        for(int x = 0; x < c.cols - 1; ++x){
            c.at(y, x) = A.at(y, x + 1) - A.at(y, x);
        }
    }
    int x = A.cols - 1;
    for(int y = 0; y < c.rows; ++y){
        c.at(y, x) = A.at(y, x) - A.at(y, x);
    }

    return c;
}

matrixus_t divY2(const matrixus_t &A)
{
    matrixus_t c = matrixus_t::zeros(A.rows, A.cols);

#pragma omp parallel for
    for(int y = 1; y < c.rows; ++y){
        for(int x = 0; x < c.cols; ++x){
            c.at(y, x) = A.at(y, x) - A.at(y - 1, x);
        }
    }

    return c;
}

matrixus_t divX2(const matrixus_t &A)
{
    matrixus_t c = matrixus_t::zeros(A.rows, A.cols);

#pragma omp parallel for
    for(int y = 0; y < c.rows; ++y){
        for(int x = 1; x < c.cols; ++x){
            c.at(y, x) =  A.at(y, x) - A.at(y, x - 1);
        }
    }

    return c;
}

/////////////////////////

TVDenoiser::TVDenoiser()
{

}

void TVDenoiser::denoise(const matrixus_t &In, matrixus_t &Out, float lambda, int niter)
{
    matrixus_t nim = In / mMaximum;
    matrixus_t u = nim;

    matrixus_t dx = divX(u);
    matrixus_t dy = divY(u);

    matrixus_t px = dx, py = dy, msqrt, normep, div, v;

    float L2 = 8;
    float tau = 0.02;
    float sigma = 1 / (L2 * tau);
    float theta = 1;
    float lt = lambda * tau;

    matrixus_t unew = matrixus_t::zeros(u.size());

    for(int i = 0; i < niter; ++i){
        dx = divX(u);
        dy = divY(u);

        px = px + sigma * dx;
        py = py + sigma * dy;

        sqrt(px.mul(px) + py.mul(py), msqrt);
        normep = max(matrixus_t::ones(px.size()), msqrt);
        px = px / normep;
        py = py / normep;

        div = divX2(px);
        div = div + divY2(py);

        v = u + tau * div;

        unew.forEach([&](float& p, const int pos[]){
            float vp = v.at(pos[0], pos[1]);
            float nimp = nim.at(pos[0], pos[1]);
            float d_v_nim = vp - nimp;
            p = (vp - lt) * (d_v_nim > lt) + (vp + lt) * (d_v_nim < -lt) + nimp * (fabsf(d_v_nim) <= lt);
        });

        u = unew + theta * (unew - u);
    }

    Out = u * mMaximum;
}
