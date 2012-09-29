//////////////////////////////////////////////////////////////////////////
// Performs the calculations for the wave simulation.  After the simulation has been
// updated, the client must copy the current solution into vertex buffers for rendering.
// This class only does the calculations, it does not do any drawing.
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>

namespace Geometry
{
    class Waves
    {
    public:
        inline Waves();
        inline ~Waves();

        inline uint32_t rows() const;
        inline uint32_t columns() const;
        inline uint32_t vertices() const;
        inline uint32_t triangles() const;

        // Returns the solution at the ith grid point.
        inline const DirectX::XMFLOAT3& operator[](const uint32_t index) const;

        void init(const uint32_t rows, const uint32_t columns, const float dx, const float dt, const float speed, const float damping);
        void update(const float dt);
        void disturb(const uint32_t i, const uint32_t j, const float magnitude);

    private:
        uint32_t mRows;
        uint32_t mColumns;

        uint32_t mVertices;
        uint32_t mTriangles;

        // Simulation constants we can precompute.
        float mK1;
        float mK2;
        float mK3;

        float mTimeStep;
        float mSpatialStep;

        DirectX::XMFLOAT3* mPreviousSolution;
        DirectX::XMFLOAT3* mCurrentSolution;
    };

    inline Waves::Waves()
        : mRows(0)
        , mColumns(0)
        , mVertices(0)
        , mTriangles(0)
        , mK1(0.0f)
        , mK2(0.0f)
        , mK3(0.0f)
        , mTimeStep(0.0f)
        , mSpatialStep(0.0f)
        , mPreviousSolution(nullptr)
        , mCurrentSolution(nullptr)
    {

    }

    inline Waves::~Waves()
    {
        delete[] mPreviousSolution;
        delete[] mCurrentSolution;
    }

    inline uint32_t Waves::rows() const
    {
        return mRows;
    }

    inline uint32_t Waves::columns() const
    {
        return mColumns;
    }

    inline uint32_t Waves::vertices() const
    {
        return mVertices;
    }

    inline uint32_t Waves::triangles() const
    {
        return mTriangles;
    }

    // Returns the solution at the ith grid point.
    inline const DirectX::XMFLOAT3& Waves::operator[](const uint32_t index) const 
    { 
        return mCurrentSolution[index]; 
    }
}