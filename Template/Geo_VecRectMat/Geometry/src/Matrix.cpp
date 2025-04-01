#include "Matrix.h"

#include <array>
#include <cmath>
#include <stdexcept>

template<size_t N, typename T>
class Matrix<N, T>::Impl {
public:
    std::array<T, N * N> data;

    Impl() : data{} {}

    T Determinant() const {
        if constexpr (N == 2) {
            return data[0] * data[3] - data[1] * data[2]; // ad - bc
        } else if constexpr (N == 3) {
            return data[0] * (data[4] * data[8] - data[5] * data[7]) -
                   data[1] * (data[3] * data[8] - data[5] * data[6]) +
                   data[2] * (data[3] * data[7] - data[4] * data[6]);
        } else if constexpr (N == 4) {
            T det = 0;
            det += data[0] * (data[5] * (data[10] * data[15] - data[11] * data[14]) -
                             data[6] * (data[9] * data[15] - data[11] * data[13]) +
                             data[7] * (data[9] * data[14] - data[10] * data[13]));
            det -= data[1] * (data[4] * (data[10] * data[15] - data[11] * data[14]) -
                             data[6] * (data[8] * data[15] - data[11] * data[12]) +
                             data[7] * (data[8] * data[14] - data[10] * data[12]));
            det += data[2] * (data[4] * (data[9] * data[15] - data[11] * data[13]) -
                             data[5] * (data[8] * data[15] - data[11] * data[12]) +
                             data[7] * (data[8] * data[13] - data[9] * data[12]));
            det -= data[3] * (data[4] * (data[9] * data[14] - data[10] * data[13]) -
                             data[5] * (data[8] * data[14] - data[10] * data[12]) +
                             data[6] * (data[8] * data[13] - data[9] * data[12]));
            return det;
        }
        return T{};
    }
};

template<size_t N, typename T>
Matrix<N, T>::Matrix() : pImpl(new Impl()) {}

template<size_t N, typename T>
template<typename>
Matrix<N, T>::Matrix(T m00, T m01, T m10, T m11) : pImpl(new Impl()) {
    pImpl->data[0] = m00;
    pImpl->data[1] = m01;
    pImpl->data[2] = m10;
    pImpl->data[3] = m11;
}

template<size_t N, typename T>
template<typename>
Matrix<N, T>::Matrix(T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22)
    : pImpl(new Impl()) {
    pImpl->data[0] = m00;
    pImpl->data[1] = m01;
    pImpl->data[2] = m02;
    pImpl->data[3] = m10;
    pImpl->data[4] = m11;
    pImpl->data[5] = m12;
    pImpl->data[6] = m20;
    pImpl->data[7] = m21;
    pImpl->data[8] = m22;
}

template<size_t N, typename T>
template<typename>
Matrix<N, T>::Matrix(T m00, T m01, T m02, T m03, T m10, T m11, T m12, T m13,
                     T m20, T m21, T m22, T m23, T m30, T m31, T m32, T m33)
    : pImpl(new Impl()) {
    pImpl->data[0] = m00;
    pImpl->data[1] = m01;
    pImpl->data[2] = m02;
    pImpl->data[3] = m03;
    pImpl->data[4] = m10;
    pImpl->data[5] = m11;
    pImpl->data[6] = m12;
    pImpl->data[7] = m13;
    pImpl->data[8] = m20;
    pImpl->data[9] = m21;
    pImpl->data[10] = m22;
    pImpl->data[11] = m23;
    pImpl->data[12] = m30;
    pImpl->data[13] = m31;
    pImpl->data[14] = m32;
    pImpl->data[15] = m33;
}

template<size_t N, typename T>
Matrix<N, T>::Matrix(const Matrix& other) 
    : pImpl(new Impl(*other.pImpl)) {}

template<size_t N, typename T>
Matrix<N, T>& Matrix<N, T>::operator=(const Matrix& other) {
    if (this != &other) {
        delete pImpl;
        pImpl = new Impl(*other.pImpl);
    }
    return *this;
}

template<size_t N, typename T>
Matrix<N, T>::Matrix(Matrix&& other) noexcept 
    : pImpl(other.pImpl) {
    other.pImpl = nullptr;
}

template<size_t N, typename T>
Matrix<N, T>& Matrix<N, T>::operator=(Matrix&& other) noexcept {
    if (this != &other) {
        delete pImpl;
        pImpl = other.pImpl;
        other.pImpl = nullptr;
    }
    return *this;
}

template<size_t N, typename T>
Matrix<N, T>::~Matrix() {
    delete pImpl;
}

template<size_t N, typename T>
T Matrix<N, T>::operator()(size_t row, size_t col) const {
    if (row >= N || col >= N) {
        throw std::out_of_range("Matrix index out of range");
    }
    return pImpl->data[row * N + col];
}

template<size_t N, typename T>
T& Matrix<N, T>::operator()(size_t row, size_t col) {
    if (row >= N || col >= N) {
        throw std::out_of_range("Matrix index out of range");
    }
    return pImpl->data[row * N + col];
}

template<size_t N, typename T>
T Matrix<N, T>::Determinant() const {
    return pImpl->Determinant();
}

// 显式实例化定义
#ifdef UTILITY_EXPORTS
template class Matrix<2, float>;
template class Matrix<2, double>;
template class Matrix<3, float>;
template class Matrix<3, double>;
template class Matrix<4, float>;
template class Matrix<4, double>;

// 修复：为 double 类型的构造函数添加显式实例化
template UTILITY_API Matrix<2, float>::Matrix(float, float, float, float);
template UTILITY_API Matrix<2, double>::Matrix(double, double, double, double);
template UTILITY_API Matrix<3, float>::Matrix(float, float, float, float, float, float, float, float, float);
template UTILITY_API Matrix<3, double>::Matrix(double, double, double, double, double, double, double, double, double);
template UTILITY_API Matrix<4, float>::Matrix(float, float, float, float, float, float, float, float,
                                              float, float, float, float, float, float, float, float);
template UTILITY_API Matrix<4, double>::Matrix(double, double, double, double, double, double, double, double,
                                               double, double, double, double, double, double, double, double);
#endif