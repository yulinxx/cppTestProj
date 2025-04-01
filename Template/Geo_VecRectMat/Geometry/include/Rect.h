#pragma once

#include "UtilityAPI.h"
#include "Vec.h"

#include <cstddef>
#include <type_traits>

template<size_t N, typename T = double>
class UTILITY_API Rect
{
public:
    Rect();
    Rect(const Vector<N, T>& min, const Vector<N, T>& max);

    Rect(const Rect& other);
    Rect& operator=(const Rect& other);

    Rect(Rect&& other) noexcept;
    Rect& operator=(Rect&& other) noexcept;

    ~Rect();

    const Vector<N, T>& GetMin() const;
    const Vector<N, T>& GetMax() const;

    T Width() const;
    T Height() const;
    template<size_t M = N, typename = std::enable_if_t<M >= 3>>
    T Depth() const;

    T Area() const;
    template<size_t M = N, typename = std::enable_if_t<M >= 3>>
    T Volume() const;

private:
    class Impl;
    Impl* pImpl;
};

#ifdef UTILITY_EXPORTS
template class UTILITY_API Rect<2, float>;
template class UTILITY_API Rect<2, double>;
template class UTILITY_API Rect<3, float>;
template class UTILITY_API Rect<3, double>;
#endif

using Rect2f = Rect<2, float>;
using Rect2d = Rect<2, double>;
using Rect3f = Rect<3, float>;
using Rect3d = Rect<3, double>;