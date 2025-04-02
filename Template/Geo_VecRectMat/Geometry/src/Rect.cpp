#include "Rect.h"

#include <array>
#include <cmath>
#include <algorithm>

namespace Ut
{

template<size_t N, typename T>
class Rect<N, T>::Impl
{
public:
    Vec<N, T> min;
    Vec<N, T> max;

    Impl() : min(), max()
    {
    }
    Impl(const Vec<N, T>& min, const Vec<N, T>& max)
        : min(min), max(max)
    {
    }
};

template<size_t N, typename T>
Rect<N, T>::Rect() : pImpl(new Impl())
{
}

template<size_t N, typename T>
Rect<N, T>::Rect(const Vec<N, T>& min, const Vec<N, T>& max)
    : pImpl(new Impl(min, max))
{
}

template<size_t N, typename T>
Rect<N, T>::Rect(const Rect& other)
    : pImpl(new Impl(*other.pImpl))
{
}

template<size_t N, typename T>
Rect<N, T>& Rect<N, T>::operator=(const Rect& other)
{
    if (this != &other)
    {
        delete pImpl;
        pImpl = new Impl(*other.pImpl);
    }
    return *this;
}

template<size_t N, typename T>
Rect<N, T>::Rect(Rect&& other) noexcept
    : pImpl(other.pImpl)
{
    other.pImpl = nullptr;
}

template<size_t N, typename T>
Rect<N, T>& Rect<N, T>::operator=(Rect&& other) noexcept
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
Rect<N, T>::~Rect()
{
    delete pImpl;
}

template<size_t N, typename T>
const Vec<N, T>& Rect<N, T>::GetMin() const
{
    return pImpl->min;
}

template<size_t N, typename T>
const Vec<N, T>& Rect<N, T>::GetMax() const
{
    return pImpl->max;
}

template<size_t N, typename T>
T Rect<N, T>::Width() const
{
    return pImpl->max[0] - pImpl->min[0];
}

template<size_t N, typename T>
T Rect<N, T>::Height() const
{
    return pImpl->max[1] - pImpl->min[1];
}

template<size_t N, typename T>
template<size_t M, typename>
T Rect<N, T>::Depth() const
{
    return pImpl->max[2] - pImpl->min[2];
}

template<size_t N, typename T>
T Rect<N, T>::Area() const
{
    T area = Width() * Height();
    return area;
}

template<size_t N, typename T>
template<size_t M, typename>
T Rect<N, T>::Volume() const
{
    return Width() * Height() * Depth();
}

#ifdef UTILITY_EXPORTS
template class Rect<2, float>;
template class Rect<2, double>;
template class Rect<3, float>;
template class Rect<3, double>;

template UTILITY_API double Rect<3, double>::Depth<3, void>() const;
template UTILITY_API double Rect<3, double>::Volume<3, void>() const;
#endif

}