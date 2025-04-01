#include "Vec.h"

#include <array>
#include <cmath>
#include <stdexcept>

template<size_t N, typename T>
class Vector<N, T>::Impl
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
Vector<N, T>::Vector() : pImpl(new Impl())
{
}

template<size_t N, typename T>
template<typename>
Vector<N, T>::Vector(T x, T y) : pImpl(new Impl())
{
    pImpl->data[0] = x;
    pImpl->data[1] = y;
}

template<size_t N, typename T>
template<typename>
Vector<N, T>::Vector(T x, T y, T z) : pImpl(new Impl())
{
    pImpl->data[0] = x;
    pImpl->data[1] = y;
    pImpl->data[2] = z;
}

// 拷贝构造函数
template<size_t N, typename T>
Vector<N, T>::Vector(const Vector& other)
    : pImpl(new Impl(*other.pImpl))
{
}

// 拷贝赋值操作符
template<size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator=(const Vector& other)
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
Vector<N, T>::Vector(Vector&& other) noexcept
    : pImpl(other.pImpl)
{
    other.pImpl = nullptr;
}

// 移动赋值操作符
template<size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator=(Vector&& other) noexcept
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
Vector<N, T>::~Vector()
{
    delete pImpl;
}

template<size_t N, typename T>
T Vector<N, T>::operator[](size_t index) const
{
    return pImpl->data.at(index);
}

template<size_t N, typename T>
T& Vector<N, T>::operator[](size_t index)
{
    return pImpl->data.at(index);
}

template<size_t N, typename T>
T Vector<N, T>::Length() const
{
    return pImpl->Length();
}

template<size_t N, typename T>
T Vector<N, T>::x() const
{
    return (*this)[0];
}

template<size_t N, typename T>
T Vector<N, T>::y() const
{
    return (*this)[1];
}

template<size_t N, typename T>
template<size_t M, typename>
T Vector<N, T>::z() const
{
    return (*this)[2];
}

// 显式实例化模板类
template class UTILITY_API Vector<2, float>;
template class UTILITY_API Vector<2, double>;
template class UTILITY_API Vector<3, float>;
template class UTILITY_API Vector<3, double>;

template UTILITY_API Vector<2, float>::Vector(float, float);
template UTILITY_API Vector<3, double>::Vector(double, double, double);