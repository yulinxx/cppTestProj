#pragma once

#include "UtilityAPI.h"
#include "Vec.h"

#include <cstddef>
#include <type_traits>

namespace Ut
{

template<size_t N, typename T = double>
class UTILITY_API Matrix {
public:
    // 构造函数
    Matrix();
    
    // 初始化为 N x N 矩阵，指定所有元素
    template<typename = std::enable_if_t<N == 2>>
    Matrix(T m00, T m01, T m10, T m11);
    
    template<typename = std::enable_if_t<N == 3>>
    Matrix(T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22);
    
    template<typename = std::enable_if_t<N == 4>>
    Matrix(T m00, T m01, T m02, T m03, T m10, T m11, T m12, T m13,
           T m20, T m21, T m22, T m23, T m30, T m31, T m32, T m33);
    
    // 拷贝和移动语义
    Matrix(const Matrix& other);
    Matrix& operator=(const Matrix& other);
    Matrix(Matrix&& other) noexcept;
    Matrix& operator=(Matrix&& other) noexcept;
    
    ~Matrix();
    
    // 元素访问
    T operator()(size_t row, size_t col) const;
    T& operator()(size_t row, size_t col);
    
    // 矩阵属性
    T Determinant() const; // 行列式

    // 矩阵和向量的乘法
    template<size_t M, typename U>
    Vec<M, U> operator*(const Vec<M, U>& vec) const;
    
private:
    class Impl;
    Impl* pImpl;
};

// 显式实例化声明
#ifdef UTILITY_EXPORTS
template class UTILITY_API Matrix<2, float>;
template class UTILITY_API Matrix<2, double>;
template class UTILITY_API Matrix<3, float>;
template class UTILITY_API Matrix<3, double>;
template class UTILITY_API Matrix<4, float>;
template class UTILITY_API Matrix<4, double>;
#endif

// 类型别名
using Matrix2f = Matrix<2, float>;
using Matrix2d = Matrix<2, double>;
using Matrix3f = Matrix<3, float>;
using Matrix3d = Matrix<3, double>;
using Matrix4f = Matrix<4, float>;
using Matrix4d = Matrix<4, double>;

}