//////////////////////////////////////////////////////////////////////////
//
// Manages initialization, destruction and access to vertex and index
// buffers of the geometry that will be rendered
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <DirectXMath.h>

#include <D3D11.h>

struct IndexedBufferInfo
{
    IndexedBufferInfo();

    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
    uint32_t mBaseVertexLocation;
    uint32_t mStartIndexLocation;
    uint32_t mIndexCount;
};

struct NonIndexedBufferInfo
{
    NonIndexedBufferInfo();

    ID3D11Buffer* mVertexBuffer;
    uint32_t mBaseVertexLocation;
    uint32_t mVertexCount;
};

struct Vertex
{
    Vertex();

    Vertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& texCoord);

    Vertex(const float positionX, 
           const float positionY, 
           const float positionZ,
           const float texCoordU, 
           const float texCoordV);

    DirectX::XMFLOAT3 mPosition;
    DirectX::XMFLOAT2 mTexCoord;
};

struct GeometryBuffers
{
    GeometryBuffers();

    IndexedBufferInfo* mBufferInfo;
};

namespace GeometryBuffersUtils
{
    void init(ID3D11Device& device, GeometryBuffers& geometryBuffers);
    void destroy(GeometryBuffers& geometryBuffers);
}