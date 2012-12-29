#include "GeometryBuffersManager.h"

#include <cassert>

#include "HLSL/Vertex.h"

#include <GeometryGenerator.h>
#include <DxErrorChecker.h>
#include <MathHelper.h>

namespace 
{
    DirectX::XMFLOAT3 normal(const float x, const float z)
    {
        // n = (-df/dx, 1, -df/dz)
        const float normalX = -0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z);
        const float normalY = 1.0f;
        const float normalZ = -0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z);
        DirectX::XMFLOAT3 normal(normalX, normalY, normalZ);

        DirectX::XMVECTOR unitNormal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal));
        DirectX::XMStoreFloat3(&normal, unitNormal);

        return normal;
    }

    float height(const float x, const float z)
    {
        return 0.3f * (0.3f * z * sinf(0.1f * x) + 0.5f * x * cosf(0.1f * z));
    }
}

namespace Managers
{
    GeometryBuffersManager::NonIndexedBufferInfo* GeometryBuffersManager::mBezierSurfaceBufferInfo = nullptr;

    void GeometryBuffersManager::initAll(ID3D11Device* device)
    {
        assert(device);

        buildBezierSurfaceBuffers(device);
    }

    void GeometryBuffersManager::destroyAll()
    {
        mBezierSurfaceBufferInfo->mVertexBuffer->Release();
        delete mBezierSurfaceBufferInfo;
    }

    void GeometryBuffersManager::buildBezierSurfaceBuffers(ID3D11Device* device)
    {
        assert(device);

        // Create IndexedBufferInfo for bezier surface
        mBezierSurfaceBufferInfo = new NonIndexedBufferInfo();
        
        // Cache base vertex location and vertex count.
        mBezierSurfaceBufferInfo->mBaseVertexLocation = 0;
        mBezierSurfaceBufferInfo->mVertexCount = 16;
        
        // Compute the total number of vertices
        const uint32_t vertexCount = mBezierSurfaceBufferInfo->mVertexCount;

        // Fill vertices
        DirectX::XMFLOAT3 vertices[16] = 
        {
            // Row 0
            DirectX::XMFLOAT3(-10.0f, -10.0f, +15.0f),
            DirectX::XMFLOAT3(-5.0f,  0.0f, +15.0f),
            DirectX::XMFLOAT3(+5.0f,  0.0f, +15.0f),
            DirectX::XMFLOAT3(+10.0f, 0.0f, +15.0f), 

            // Row 1
            DirectX::XMFLOAT3(-15.0f, 0.0f, +5.0f),
            DirectX::XMFLOAT3(-5.0f,  0.0f, +5.0f),
            DirectX::XMFLOAT3(+5.0f,  20.0f, +5.0f),
            DirectX::XMFLOAT3(+15.0f, 0.0f, +5.0f), 

            // Row 2
            DirectX::XMFLOAT3(-15.0f, 0.0f, -5.0f),
            DirectX::XMFLOAT3(-5.0f,  0.0f, -5.0f),
            DirectX::XMFLOAT3(+5.0f,  0.0f, -5.0f),
            DirectX::XMFLOAT3(+15.0f, 0.0f, -5.0f), 

            // Row 3
            DirectX::XMFLOAT3(-10.0f, 10.0f, -15.0f),
            DirectX::XMFLOAT3(-5.0f,  0.0f, -15.0f),
            DirectX::XMFLOAT3(+5.0f,  0.0f, -15.0f),
            DirectX::XMFLOAT3(+25.0f, 10.0f, -15.0f)
        };

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::BezierSurfaceVertex) * vertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = vertices;
        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mBezierSurfaceBufferInfo->mVertexBuffer);
        DebugUtils::DxErrorChecker(result);
    }
}