#ifndef MATUTILS_H
#define MATUTILS_H

#include <vector>

#include "mat.h"

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

    Size size() const{
        return {cols, rows};
    }

    void resize(int rows, int cols){
        this->rows = rows;
        this->cols = cols;
        data.resize(rows * cols, 0);
    }
    void resize(const Size& sz){
        this->rows = sz.height;
        this->cols = sz.width;
        data.resize(rows * cols, 0);
    }
    void clear(){
        this->rows = 0;
        this->cols = 0;
        data.clear();
    }
    T& at(int y, int x){
        return *(reinterpret_cast<T*>(data.data()) + y * cols + x);
    }
    const T at(int y, int x)const{
        return *(reinterpret_cast<const T*>(data.data()) + y * cols + x);
    }
    T* ptr(int y = 0){
        return reinterpret_cast<T*>(data.data()) + y * cols;
    }
    const T* ptr(int y = 0) const{
        return reinterpret_cast<const T*>(data.data()) + y * cols;
    }
    T& operator[](int i){
        return data[i];
    }
    T operator[](int i) const{
        return data[i];
    }

    Matrix<T>& mul(T b){
    #pragma omp parallel for
        for(int y = 0; y < rows; ++y){
            T *dP = ptr(y);
            for(int x = 0; x < cols; ++x){
                dP[x] *= b;
            }
        }

        return *this;
    }

    Matrix<T>& mul(const Matrix<T>& B){
    #pragma omp parallel for
        for(int y = 0; y < rows; ++y){
            T *dP = ptr(y);
            const T *dB = B.ptr(y);
            for(int x = 0; x < cols; ++x){
                dP[x] *= dB[x];
            }
        }

        return *this;
    }

    Matrix<T>& div(const Matrix<T>& B){
    #pragma omp parallel for
        for(int y = 0; y < rows; ++y){
            T *dP = ptr(y);
            const T *dB = B.ptr(y);
            for(int x = 0; x < cols; ++x){
                dP[x] /= dB[x];
            }
        }

        return *this;
    }

    Matrix<T>& add(const Matrix<T>& B){
    #pragma omp parallel for
        for(int y = 0; y < rows; ++y){
            T *dP = ptr(y);
            const T *dB = B.ptr(y);
            for(int x = 0; x < cols; ++x){
                dP[x] += dB[x];
            }
        }

        return *this;
    }

    void minMaxLoc(T& Min, T& Max) const{
        const T *dP = ptr(0);
        Min = 1e+12;
        Max = -1e+12;
        for(int i = 0; i < total(); ++i){
            Min = std::min(Min, dP[i]);
            Max = std::max(Max, dP[i]);
        }
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

    template<typename F>
    void forEach(F fun){
#pragma omp parallel for
        for(int y = 0; y < rows; ++y){
            T *dP = ptr(y);
            for(int x = 0; x < cols; ++x){
                int pos[] = {y, x};
                fun(dP[x], pos);
            }
        }
    }

    Matrix<T> clone() const{
         Matrix<T> R;
         R.resize(rows, cols);
#pragma omp parallel for
        for(int y = 0; y < rows; ++y){
            const T *dO = ptr(y);
            T *dP = R.ptr(y);
            for(int x = 0; x < cols; ++x){
                dP[x] = dO[x];
            }
        }
        return R;
    }

    static Matrix<T> zeros(int rows, int cols){
        Matrix<T> ret;
        ret.resize(rows, cols);
#pragma omp parallel for
        for(int y = 0; y < ret.rows; ++y){
            T *dP = ret.ptr(y);
            for(int x = 0; x < ret.cols; ++x){
                dP[x] = 0;
            }
        }

        return ret;
    }
    static Matrix<T> zeros(const Size& sz){
        return zeros(sz.height, sz.width);
    }

    static Matrix<T> ones(const Size& sz){
        Matrix<T> ret;
        ret.resize(sz.height, sz.width);

#pragma omp parallel for
        for(int y = 0; y < ret.rows; ++y){
            T *dP = ret.ptr(y);
            for(int x = 0; x < ret.cols; ++x){
                dP[x] = 1;
            }
        }
        return ret;
    }
};

template <typename T>
Matrix<T> operator/(const Matrix<T>& A, T b)
{
    Matrix<T> R;
    R.resize(A.size());

#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dP = A.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = 1.f * dP[x] / b;
        }
    }

    return R;
}

template <typename T>
Matrix<T> operator/(const Matrix<T>& A, const Matrix<T>& B)
{
    Matrix<T> R;
    R.resize(A.size());

#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dA = A.ptr(y);
        const T *dB = B.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = 1.f * dA[x] / dB[x];
        }
    }

    return R;
}

template <typename T>
Matrix<T> operator+(const Matrix<T>& A, T b)
{
    Matrix<T> R;
    R.resize(A.size());

#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dP = A.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = dP[x] + b;
        }
    }

    return R;
}

template <typename T>
Matrix<T> operator+(const Matrix<T>& A, const Matrix<T>& B)
{
    Matrix<T> R;
    R.resize(A.size());

#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dA = A.ptr(y);
        const T *dB = B.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = dA[x] + dB[x];
        }
    }

    return R;
}


template <typename T>
Matrix<T> operator-(const Matrix<T>& A, const Matrix<T>& B)
{
    Matrix<T> R;
    R.resize(A.size());

#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dA = A.ptr(y);
        const T *dB = B.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = dA[x] - dB[x];
        }
    }

    return R;
}

template <typename T>
Matrix<T> operator*(const Matrix<T>& A, T b)
{
    Matrix<T> R;
    R.resize(A.size());

#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dP = A.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = dP[x] * b;
        }
    }

    return R;
}

template <typename T>
Matrix<T> operator*(T b, const Matrix<T>& A)
{
    Matrix<T> R;
    R.resize(A.size());

#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dP = A.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = dP[x] * b;
        }
    }

    return R;
}

template <typename T>
void sqrt(const Matrix<T>& A, Matrix<T>& R)
{
    R.resize(A.size());
#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dP = A.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = sqrt(dP[x]);
        }
    }
}

template <typename T>
void norml2(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& R)
{
    R.resize(A.size());
#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dA = A.ptr(y);
        const T *dB = B.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = sqrt(dA[x] * dA[x] + dB[x] * dB[x]);
        }
    }
}

template <typename T>
Matrix<T> max(const Matrix<T>& A, const Matrix<T>& B)
{
    Matrix<T> R;
    R.resize(A.size());

#pragma omp parallel for
    for(int y = 0; y < A.rows; ++y){
        const T *dA = A.ptr(y);
        const T *dB = B.ptr(y);
        T *dO = R.ptr(y);
        for(int x = 0; x < A.cols; ++x){
            dO[x] = fmaxf(dA[x], dB[x]);
        }
    }

    return R;
}

typedef Matrix<float> matrixus_t;
typedef std::vector< float > vectorus_t;

#endif // MATUTILS_H
