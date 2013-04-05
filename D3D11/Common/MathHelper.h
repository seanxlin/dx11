#pragma once

#include <DirectXMath.h>

namespace MathHelper
{
    template<typename T>
    T computeMax(const T& a, 
                 const T& b)
    {
        return a > b ? a : b;
    }

    template<typename T>
    T computeMin(const T& a, 
                 const T& b)
    {
        return a < b ? a : b;
    }

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

    DirectX::XMMATRIX inverseTranspose(DirectX::CXMMATRIX matrix);

    float height(const float x, 
                 const float z);

    float angleFromXY(const float x, 
                      const float y);
}