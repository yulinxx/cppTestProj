#pragma once
#include "UtilityAPI.h"

#include <memory>
#include <vector>

#include <cstddef>
#include <type_traits>

namespace Ut
{

template<size_t N, typename T = double>
class UTILITY_API Vec
{
public:
    Vec();

    template<typename = std::enable_if_t<N == 2>>
    Vec(T x, T y);

    template<typename = std::enable_if_t<N == 3>>
    Vec(T x, T y, T z);

    // 添加拷贝和移动构造函数/赋值操作符
    Vec(const Vec& other);
    Vec& operator=(const Vec& other);
    Vec(Vec&& other) noexcept;
    Vec& operator=(Vec&& other) noexcept;

    ~Vec();

    T operator[](size_t index) const;
    T& operator[](size_t index);
    T Length() const;

    T x() const;
    T y() const;
    template<size_t M = N, typename = std::enable_if_t<M >= 3>>
    T z() const;

    // 重载运算符
    Vec operator+(const Vec& other) const;
    Vec operator-(const Vec& other) const;

    // 点积函数
    T dot(const Vec& other) const;

    // 叉积函数
    template<size_t M = N, typename = std::enable_if_t<M == 2>>
    T cross(const Vec& other) const;  // 2D 返回标量

    template<size_t M = N, typename = std::enable_if_t<M == 3>>
    Vec cross(const Vec& other) const;  // 3D 返回向量

    // 旋转方法声明
    template<typename = std::enable_if_t<N == 2>>
    Vec rotate(float angle) const;

private:
    class Impl;
    Impl* pImpl;
};

using Vec2f = Vec<2, float>;
using Vec2d = Vec<2, double>;
using Vec3f = Vec<3, float>;
using Vec3d = Vec<3, double>;

} // namespace Ut