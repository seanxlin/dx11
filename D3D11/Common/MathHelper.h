#pragma once

#include <cstdlib>
#include <DirectXMath.h>

namespace Utils
{
    class MathHelper
    {
    public:
        template<typename T>
        static inline T computeMax(const T& a, const T& b);

        template<typename T>
        static inline T computeMin(const T& a, const T& b);

        template<typename T>
        static inline T clamp(const T& x, const T& low, const T& high);

        // Returns random float in [0, 1).
        static inline float randomFloat();

        // Returns random float in [a, b).
        static inline float randomFloat(const float leftLimit, const float righLimit);

        static inline DirectX::XMMATRIX inverseTranspose(DirectX::CXMMATRIX matrix);

        static float angleFromXY(const float x, const float y);

    private:
        ~MathHelper();
        MathHelper(const MathHelper& copy);
        const MathHelper& operator=(const MathHelper& copy);
    };

    template<typename T>
    inline T MathHelper::computeMax(const T& a, const T& b)
    {
        return a > b ? a : b;
    }

    template<typename T>
    inline T MathHelper::computeMin(const T& a, const T& b)
    {
        return a < b ? a : b;
    }

    template<typename T>
    inline T MathHelper::clamp(const T& x, const T& low, const T& high)
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

    inline DirectX::XMMATRIX MathHelper::inverseTranspose(DirectX::CXMMATRIX matrix) 
    { 
        DirectX::XMMATRIX copy = matrix; 
        copy.r[3] = DirectX::XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f); 
        DirectX::XMVECTOR determinant = DirectX::XMMatrixDeterminant(matrix); 
        
        return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&determinant, matrix)); 
    }

}