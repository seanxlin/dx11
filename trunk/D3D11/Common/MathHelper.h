#pragma once

#include <DirectXMath.h>

namespace MathUtils
{
    template<typename T>
    T clamp(const T& x, const T& low, const T& high)
    {
        return x < low ? low : (x > high ? high : x); 
    }

    float angleFromXY(const float x, const float y);
}