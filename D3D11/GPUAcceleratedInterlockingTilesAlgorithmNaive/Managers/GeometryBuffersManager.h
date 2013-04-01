#pragma once

#include <cstdint>
#include <DirectXMath.h>

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

        struct Vertex
        {
            Vertex() { }

            Vertex(const DirectX::XMFLOAT3& position,  
                const DirectX::XMFLOAT2& texCoord)
                : mPosition(position)
                , mTexCoord(texCoord)
            {
            }

            Vertex(const float positionX, 
                const float positionY, 
                const float positionZ,
                const float texCoordU, 
                const float texCoordV)
                : mPosition(positionX, positionY, positionZ)
                , mTexCoord(texCoordU, texCoordV)
            {
            }

            DirectX::XMFLOAT3 mPosition;
            DirectX::XMFLOAT2 mTexCoord;
        };

    public:
        static void initAll(ID3D11Device& device);
        static void destroyAll();

        static IndexedBufferInfo* mTerrainBufferInfo;

    private:
        GeometryBuffersManager();
        ~GeometryBuffersManager();
        GeometryBuffersManager(const GeometryBuffersManager& geometryBuffersManager);
        const GeometryBuffersManager& operator=(const GeometryBuffersManager& geometryBuffersManager);

        static void buildTerrainBuffers(ID3D11Device& device);
    };
}