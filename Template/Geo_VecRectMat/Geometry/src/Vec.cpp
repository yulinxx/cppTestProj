#include "Vec.h"

#include <array>
#include <cmath>
#include <stdexcept>

namespace Ut
{
#define PI 3.14159265358979323846f

    template<size_t N, typename T>
    class Vec<N, T>::Impl
    {
    public:
        std::array<T, N> data;

        Impl() : data{}
        {
        }

        T Length() const
        {
            T sum = 0;
            for (const auto& val : data)
            {
                sum += val * val;
            }
            return std::sqrt(sum);
        }
    };

    template<size_t N, typename T>
    Vec<N, T>::Vec() : pImpl(new Impl())
    {
    }

    template<size_t N, typename T>
    template<typename>
    Vec<N, T>::Vec(T x, T y) : pImpl(new Impl())
    {
        pImpl->data[0] = x;
        pImpl->data[1] = y;
    }


    template<size_t N, typename T>
    template<typename>
    Vec<N, T>::Vec(T x, T y, T z) : pImpl(new Impl())
    {
        pImpl->data[0] = x;
        pImpl->data[1] = y;
        pImpl->data[2] = z;
    }

    // 拷贝构造函数
    template<size_t N, typename T>
    Vec<N, T>::Vec(const Vec& other)
        : pImpl(new Impl(*other.pImpl))
    {
    }

    // 拷贝赋值操作符
    template<size_t N, typename T>
    Vec<N, T>& Vec<N, T>::operator=(const Vec& other)
    {
        if (this != &other)
        {
            delete pImpl;
            pImpl = new Impl(*other.pImpl);
        }
        return *this;
    }

    // 移动构造函数
    template<size_t N, typename T>
    Vec<N, T>::Vec(Vec&& other) noexcept
        : pImpl(other.pImpl)
    {
        other.pImpl = nullptr;
    }

    // 移动赋值操作符
    template<size_t N, typename T>
    Vec<N, T>& Vec<N, T>::operator=(Vec&& other) noexcept
    {
        if (this != &other)
        {
            delete pImpl;
            pImpl = other.pImpl;
            other.pImpl = nullptr;
        }
        return *this;
    }

    template<size_t N, typename T>
    Vec<N, T>::~Vec()
    {
        delete pImpl;
    }

    template<size_t N, typename T>
    T Vec<N, T>::operator[](size_t index) const
    {
        return pImpl->data.at(index);
    }

    template<size_t N, typename T>
    T& Vec<N, T>::operator[](size_t index)
    {
        return pImpl->data.at(index);
    }

    template<size_t N, typename T>
    Vec<N, T> Vec<N, T>::operator+(const Vec& other) const
    {
        Vec result;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] = (*this)[i] + other[i];
        }
        return result;
    }

    template<size_t N, typename T>
    Vec<N, T> Vec<N, T>::operator-(const Vec& other) const
    {
        Vec result;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] = (*this)[i] - other[i];
        }
        return result;
    }

    template<size_t N, typename T>
    T Vec<N, T>::dot(const Vec& other) const
    {
        T result = T();
        for (size_t i = 0; i < N; ++i)
        {
            result += (*this)[i] * other[i];
        }
        return result;
    }

    template<size_t N, typename T>
    T Vec<N, T>::Length() const
    {
        return pImpl->Length();
    }

    template<size_t N, typename T>
    T Vec<N, T>::x() const
    {
        return (*this)[0];
    }

    template<size_t N, typename T>
    T Vec<N, T>::y() const
    {
        return (*this)[1];
    }

    template<size_t N, typename T>
    template<size_t M, typename>
    T Vec<N, T>::z() const
    {
        return (*this)[2];
    }

    // 2D 叉积实现（返回标量）
    template<size_t N, typename T>
    template<size_t M, typename>
    T Vec<N, T>::cross(const Vec& other) const
    {
        return (*this)[0] * other[1] - (*this)[1] * other[0];
    }

    // 3D 叉积实现（返回向量）
    template<size_t N, typename T>
    template<size_t M, typename>
    Vec<N, T> Vec<N, T>::cross(const Vec& other) const
    {
        Vec result;
        result[0] = (*this)[1] * other[2] - (*this)[2] * other[1];
        result[1] = (*this)[2] * other[0] - (*this)[0] * other[2];
        result[2] = (*this)[0] * other[1] - (*this)[1] * other[0];
        return result;
    }

    // 显式实例化模板类
    template class UTILITY_API Vec<2, float>;
    template class UTILITY_API Vec<2, double>;
    template class UTILITY_API Vec<3, float>;
    template class UTILITY_API Vec<3, double>;

    template UTILITY_API Vec<2, float>::Vec(float, float);
    template UTILITY_API Vec<2, double>::Vec(double, double);
    template UTILITY_API Vec<3, float>::Vec(float, float, float);
    template UTILITY_API Vec<3, double>::Vec(double, double, double);

    // 显式实例化 cross 函数
    template UTILITY_API float Vec<2, float>::cross<2, void>(const Vec<2, float>&) const;
    template UTILITY_API double Vec<2, double>::cross<2, void>(const Vec<2, double>&) const;
    template UTILITY_API Vec<3, float> Vec<3, float>::cross<3, void>(const Vec<3, float>&) const;
    template UTILITY_API Vec<3, double> Vec<3, double>::cross<3, void>(const Vec<3, double>&) const;



template<size_t N, typename T>
template<typename>
Vec<N, T> Vec<N, T>::rotate(float angle) const
{
    static_assert(N == 2, "rotate method is only available for 2D vectors.");

    // 将角度转换为弧度
    float rad = angle * (PI / 180.0f);
    float cosAngle = std::cos(rad);
    float sinAngle = std::sin(rad);

    // 获取当前向量的 x 和 y 分量
    T x = (*this)[0];
    T y = (*this)[1];

    // 应用旋转公式
    T newX = x * cosAngle - y * sinAngle;
    T newY = x * sinAngle + y * cosAngle;

    // 创建并返回旋转后的向量
    return Vec<N, T>(newX, newY);
}


} // namespace Ut
