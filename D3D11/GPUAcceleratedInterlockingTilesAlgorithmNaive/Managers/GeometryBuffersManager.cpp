#include "GeometryBuffersManager.h"

#include <cassert>

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
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mFloorBufferInfo = nullptr;

    void GeometryBuffersManager::initAll(ID3D11Device& device)
    {
        buildFloorBuffers(device);
    }

    void GeometryBuffersManager::destroyAll()
    {
        mFloorBufferInfo->mVertexBuffer->Release();
        mFloorBufferInfo->mIndexBuffer->Release();
        delete mFloorBufferInfo;
    }
    
    void GeometryBuffersManager::buildFloorBuffers(ID3D11Device& device)
    {
        // Create IndexedBufferInfo for floor
        mFloorBufferInfo = new IndexedBufferInfo();

        // 
        // Calculate vertices and indices
        // Cache vertex offset, index count and offset
        //
        Geometry::GeometryGenerator::MeshData grid;
        Geometry::GeometryGenerator::createGrid(400.0f, 400.0f, 100, 100, grid);

        // Cache base vertex location
        mFloorBufferInfo->mBaseVertexLocation = 0;

        // Cache the index count
        mFloorBufferInfo->mIndexCount = static_cast<uint32_t> (grid.mIndices.size());

        // Cache the starting index
        mFloorBufferInfo->mStartIndexLocation = 0; 

        // Compute the total number of vertices
        const uint32_t totalVertexCount = static_cast<uint32_t> (grid.mVertices.size());

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::GeometryGenerator::Vertex) * totalVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &grid.mVertices[0];
        HRESULT result = device.CreateBuffer(&vertexBufferDesc, &initData, &mFloorBufferInfo->mVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        // Fill and Create index buffer
        const uint32_t totalIndexCount = mFloorBufferInfo->mIndexCount;

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * totalIndexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &grid.mIndices[0];
        result = device.CreateBuffer(&indexBufferDesc, &initData, &mFloorBufferInfo->mIndexBuffer);
        DebugUtils::DxErrorChecker(result);
    }
}