#include "Waves.h"

#include <algorithm>
#include <cassert>

namespace Geometry
{
    void Waves::init(const uint32_t rows, const uint32_t columns, const float dx, 
        const float dt, const float speed, const float damping)
    {
        // Initialize rows, columns, vertices and triangles counts.
        mRows = rows;
        mColumns = columns;
        const uint32_t rowsPerColumns = rows * columns;
        mVertices = rowsPerColumns;
        mTriangles = (rows - 1) * (columns - 1) * 2;

        // Initialize time and spatial steps
        mTimeStep = dt;
        mSpatialStep = dx;

        // Initialize simulation constants
        const float d = damping * dt + 2.0f;
        const float e = (speed * speed) * (dt * dt) / (dx * dx);
        mK1 = (damping * dt - 2.0f) / d;
        mK2 = (4.0f - 8.0f * e) / d;
        mK3 = (2.0f * e) / d;

        // In case Init() called again.
        delete[] mPreviousSolution;
        delete[] mCurrentSolution;

        mPreviousSolution = new DirectX::XMFLOAT3[rowsPerColumns];
        mCurrentSolution = new DirectX::XMFLOAT3[rowsPerColumns];

        // Generate grid vertices in system memory.
        const float halfWidth = (columns - 1) * dx * 0.5f;
        const float halfDepth = (columns - 1) * dx * 0.5f;
        for(size_t i = 0; i < rows; ++i)
        {
            const float z = halfDepth - i * dx;
            for(size_t j = 0; j < columns; ++j)
            {
                const float x = -halfWidth + j * dx;

                const uint32_t currentIndex = static_cast<uint32_t> (i * columns + j); 
                mPreviousSolution[currentIndex] = DirectX::XMFLOAT3(x, 0.0f, z);
                mCurrentSolution[currentIndex] = DirectX::XMFLOAT3(x, 0.0f, z);
            }
        }
    }

    void Waves::update(const float dt)
    {
        static float t = 0;

        // Accumulate time.
        t += dt;

        // Only update the simulation at the specified time step.
        if( t >= mTimeStep )
        {
            // Only update interior points; we use zero boundary conditions.
            for(size_t i = 1; i < mRows - 1; ++i)
            {
                for(size_t j = 1; j < mColumns - 1; ++j)
                {
                    // After this update we will be discarding the old previous
                    // buffer, so overwrite that buffer with the new update.
                    // Note how we can do this in place (read/write to same element) 
                    // because we won't need prev_ij again and the assignment happens last.

                    // Note j indexes x and i indexes z: h(x_j, z_i, t_k)
                    // Moreover, our +z axis goes "down"; this is just to 
                    // keep consistent with our row indices going down.

                    mPreviousSolution[i * mColumns + j].y = 
                        mK1 * mPreviousSolution[i * mColumns + j].y +
                        mK2 * mCurrentSolution[i * mColumns + j].y +
                        mK3 * (mCurrentSolution[(i + 1) * mColumns + j].y + 
                        mCurrentSolution[(i - 1) * mColumns + j].y + 
                        mCurrentSolution[i * mColumns + j + 1].y + 
                        mCurrentSolution[i * mColumns + j - 1].y);
                }
            }

            // We just overwrote the previous buffer with the new data, so
            // this data needs to become the current solution and the old
            // current solution becomes the new previous solution.
            std::swap(mPreviousSolution, mCurrentSolution);

            t = 0.0f; // reset time
        }
    }

    void Waves::disturb(const uint32_t i, const uint32_t j, const float magnitude)
    {
        // Don't disturb boundaries.
        assert(i > 1 && i < mRows - 2);
        assert(j > 1 && j < mColumns - 2);

        const float halfMagnitude = 0.5f * magnitude;

        // Disturb the ijth vertex height and its neighbors.
        mCurrentSolution[i * mColumns + j].y += magnitude;
        mCurrentSolution[i * mColumns + j + 1].y += halfMagnitude;
        mCurrentSolution[i * mColumns + j - 1].y += halfMagnitude;
        mCurrentSolution[(i + 1) * mColumns + j].y += halfMagnitude;
        mCurrentSolution[(i - 1) * mColumns + j].y += halfMagnitude;
    }
}
