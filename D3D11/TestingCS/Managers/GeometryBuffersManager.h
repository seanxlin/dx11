#pragma once

#include <cstdint>
#include <DirectXMath.h>

#include <D3D11.h>

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
           const DirectX::XMFLOAT3& normal,
           const DirectX::XMFLOAT2& texCoord)
        : mPosition(position)
        , mNormal(normal)
        , mTexCoord(texCoord)
    {
    }

    DirectX::XMFLOAT3 mPosition;
    DirectX::XMFLOAT3 mNormal;
    DirectX::XMFLOAT2 mTexCoord;
};

struct GeometryBuffers
{
    GeometryBuffers()
        : mBufferInfo(nullptr)
    {

    }

    IndexedBufferInfo* mBufferInfo;
};

namespace GeometryBuffersUtils
{
    void initAll(ID3D11Device& device, GeometryBuffers& geometryBuffers);
    void destroyAll(GeometryBuffers& geometryBuffers);
}