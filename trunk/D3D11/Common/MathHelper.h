//////////////////////////////////////////////////////////////////////////
//
// Useful mathematics functions
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>

namespace MathHelper
{
    template<typename T>
    T computeMax(const T& a, const T& b) { return a > b ? a : b; }

    template<typename T>
    T computeMin(const T& a, const T& b) { return a < b ? a : b; }

    template<typename T>
    T clamp(const T& x, 
            const T& low, 
            const T& high)
    {
        return x < low ? low : (x > high ? high : x); 
    }

    // Returns random float in [0, 1).
    float randomFloat();

    // Returns random float in [a, b).
    float randomFloat(const float leftLimit, 
                      const float righLimit);

    DirectX::XMMATRIX inverseTranspose(const DirectX::XMMATRIX& matrix);

    // Based on x and z coordinates, it returns
    // y coordinates which represents height
    float height(const float x, const float z);

    // Euler angle from positive x axis
    // to the segment that goes from (0, 0) --> (x, y)
    float angle(const float x, const float y);
}