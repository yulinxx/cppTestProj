#pragma once
#include "UtilityAPI.h"

#include <memory>
#include <vector>

#include <cstddef>
#include <type_traits>

template<size_t N, typename T = double>
class UTILITY_API Vector
{
public:
    Vector();

    template<typename = std::enable_if_t<N == 2>>
    Vector(T x, T y);

    template<typename = std::enable_if_t<N == 3>>
    Vector(T x, T y, T z);

    // 添加拷贝和移动构造函数/赋值操作符
    Vector(const Vector& other);
    Vector& operator=(const Vector& other);
    Vector(Vector&& other) noexcept;
    Vector& operator=(Vector&& other) noexcept;

    ~Vector();

    T operator[](size_t index) const;
    T& operator[](size_t index);
    T Length() const;

    T x() const;
    T y() const;
    template<size_t M = N, typename = std::enable_if_t<M >= 3>>
    T z() const;

private:
    class Impl;
    Impl* pImpl;
};

using Vector2f = Vector<2, float>;
using Vector2d = Vector<2, double>;
using Vector3f = Vector<3, float>;
using Vector3d = Vector<3, double>;