#pragma once

#include <cstdint>

#include <D3D11.h>

namespace Managers
{
    class GeometryBuffersManager
    {
    public:
        struct IndexedBufferInfo
        {
            IndexedBufferInfo()
                : mVertexBuffer(nullptr)
                , mIndexBuffer(nullptr)
                , mBaseVertexLocation(0)
                , mStartIndexLocation(0)
                , mIndexCount(0)
            {

            }

            ID3D11Buffer* mVertexBuffer;
            ID3D11Buffer* mIndexBuffer;
            uint32_t mBaseVertexLocation;
            uint32_t mStartIndexLocation;
            uint32_t mIndexCount;
        };

        struct NonIndexedBufferInfo
        {
            NonIndexedBufferInfo()
                : mVertexBuffer(nullptr)
                , mBaseVertexLocation(0)
                , mVertexCount(0)
            {

            }

            ID3D11Buffer* mVertexBuffer;
            uint32_t mBaseVertexLocation;
            uint32_t mVertexCount;
        };

    public:
        static void initAll(ID3D11Device* device);
        static void destroyAll();

        static IndexedBufferInfo* mLandBufferInfo;
        static IndexedBufferInfo* mSkyBufferInfo;
        static IndexedBufferInfo* mSphereBufferInfo;

    private:
        GeometryBuffersManager();
        ~GeometryBuffersManager();
        GeometryBuffersManager(const GeometryBuffersManager& geometryBuffersManager);
        const GeometryBuffersManager& operator=(const GeometryBuffersManager& geometryBuffersManager);

        static void buildLandBuffers(ID3D11Device* device);
        static void buildSkyBuffers(ID3D11Device* device);
        static void buildSphereBuffers(ID3D11Device* device);
    };
}