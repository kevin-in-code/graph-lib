
#pragma once

/**
 * SmallFunctions.hpp
 * Purpose: Various small utility functions.
 *
 * @author Kevin A. Naudé
 * @version 1.1
 */

#include <limits>
#include <math.h>

namespace kn
{

	double roundEven(double x)
	{
		return round(x * 0.5) * 2.0;
	}

    template<typename T, T x, T y, bool done>
    struct ConstSqrtNewton
    {
    private:
        static const T q = y / x;
        static const T x1 = (x/2 + q/2);
    public:
        static const T value = ConstSqrtNewton<T, x1, y, (x1 == x) || (x1 == q)>::value;
    };

    template<typename T, T x, T y>
    struct ConstSqrtNewton<T, x, y, true>
    {
        static const T value = x;
    };

    template<typename T, T n>
    struct ConstSqrt : ConstSqrtNewton<T, 1, n, false> { };
    
    
    template<typename T>
    struct HalfCardinality
    {
    public:
        static const T value = 1 + ConstSqrt<T, std::numeric_limits<T>::max()>::value;
    };

};
