#include "MathHelper.h"

#include <cstdlib>

namespace Utils
{
    namespace MathHelper
    {
        // Returns random float in [0, 1).
        float randomFloat()
        {
            return static_cast<float> (std::rand()) / static_cast<float> (RAND_MAX);
        }

        // Returns random float in [a, b).
        float randomFloat(const float leftLimit, const float righLimit)
        {
            return leftLimit + randomFloat() * (righLimit - leftLimit);
        }

        DirectX::XMMATRIX inverseTranspose(DirectX::CXMMATRIX matrix) 
        { 
            DirectX::XMMATRIX copy = matrix; 
            copy.r[3] = DirectX::XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f); 
            DirectX::XMVECTOR determinant = DirectX::XMMatrixDeterminant(matrix); 

            return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&determinant, matrix)); 
        }

        float height(const float x, const float z)
        {
            return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
        }

        float angleFromXY(const float x, const float y)
        {
            float theta = 0.0f;

            // Quadrant I or IV
            if (x >= 0.0f) 
            {
                // If x = 0, then atanf(y/x) = +pi/2 if y > 0
                //                atanf(y/x) = -pi/2 if y < 0
                theta = atanf(y / x); // in [-pi/2, +pi/2]

                if (theta < 0.0f)
                {
                    theta += 2.0f * DirectX::XM_PI; // in [0, 2*pi).
                }            
            }

            // Quadrant II or III
            else   
            {
                theta = atanf(y / x) + DirectX::XM_PI; // in [0, 2*pi).
            }

            return theta;
        }
    }
}