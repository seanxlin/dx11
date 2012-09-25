#pragma once

#include <cstdlib>
#include <DirectXMath.h>

namespace Utils
{
    class MathHelper
    {
    public:
        template<typename T>
        static inline T clamp(const T& x, const T& low, const T& high);

        // Returns random float in [0, 1).
        static inline float randomFloat();

        // Returns random float in [a, b).
        static inline float randomFloat(const float leftLimit, const float righLimit);

        static float angleFromXY(const float x, const float y);

    private:
        ~MathHelper();
        MathHelper(const MathHelper& copy);
        const MathHelper& operator=(const MathHelper& copy);
    };

    template<typename T>
    T MathHelper::clamp(const T& x, const T& low, const T& high)
    {
        return x < low ? low : (x > high ? high : x); 
    }

    // Returns random float in [0, 1).
    float MathHelper::randomFloat()
    {
        return static_cast<float> (std::rand()) / static_cast<float> (RAND_MAX);
    }

    // Returns random float in [a, b).
    float MathHelper::randomFloat(const float leftLimit, const float righLimit)
    {
        return leftLimit + randomFloat() * (righLimit - leftLimit);
    }
}