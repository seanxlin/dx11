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
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mCylinderBufferInfo =  nullptr;
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mFloorBufferInfo = nullptr;
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mSphereBufferInfo = nullptr;
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mBoxBufferInfo = nullptr;

    void GeometryBuffersManager::initAll(ID3D11Device* device)
    {
        assert(device);

        buildCylinderBuffers(device);
        buildFloorBuffers(device);
        buildSphereBuffers(device);
        buildBoxBuffers(device);
    }

    void GeometryBuffersManager::destroyAll()
    {
        mCylinderBufferInfo->mVertexBuffer->Release();
        mCylinderBufferInfo->mIndexBuffer->Release();
        delete mCylinderBufferInfo;

        mFloorBufferInfo->mVertexBuffer->Release();
        mFloorBufferInfo->mIndexBuffer->Release();
        delete mFloorBufferInfo;

        mSphereBufferInfo->mVertexBuffer->Release();
        mSphereBufferInfo->mIndexBuffer->Release();
        delete mSphereBufferInfo;

        mBoxBufferInfo->mVertexBuffer->Release();
        mBoxBufferInfo->mIndexBuffer->Release();
        delete mBoxBufferInfo;
    }

    void GeometryBuffersManager::buildBoxBuffers(ID3D11Device* device)
    {
        assert(device);

        mBoxBufferInfo = new IndexedBufferInfo();

        // Set base vertex location
        mBoxBufferInfo->mBaseVertexLocation = 0;

        // Fill vertices to insert in the vertex buffer
        const float width = 15.0f;
        const float height = 15.0f;
        const float depth = 15.0f;
        Geometry::MeshData box;
        Geometry::GeometryGenerator::createBox(width, height, depth, box);

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = static_cast<uint32_t> (sizeof(Geometry::VertexData) * box.mVertices.size());
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &box.mVertices[0];

        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mBoxBufferInfo->mVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        // Set index count
        mBoxBufferInfo->mIndexCount = static_cast<uint32_t> (box.mIndices.size());

        // Create and fill index buffer
        D3D11_BUFFER_DESC ibd;
        ibd.Usage = D3D11_USAGE_IMMUTABLE;
        ibd.ByteWidth = sizeof(uint32_t) * mBoxBufferInfo->mIndexCount;
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;
        ibd.StructureByteStride = 0;
        ibd.MiscFlags = 0;

        initData.pSysMem = &box.mIndices[0];

        result = device->CreateBuffer(&ibd, &initData, &mBoxBufferInfo->mIndexBuffer);
        DebugUtils::DxErrorChecker(result);
    }

    void GeometryBuffersManager::buildSphereBuffers(ID3D11Device* device)
    {
        assert(device);

        mSphereBufferInfo = new IndexedBufferInfo();

        // Set base vertex location
        mSphereBufferInfo->mBaseVertexLocation = 0;

        // Fill vertices to insert in the vertex buffer
        const float radius = 15.0f;
        const uint32_t sliceCount = 20;
        const uint32_t stackCount = 20;
        Geometry::MeshData sphere;
        Geometry::GeometryGenerator::createSphere(radius, sliceCount, stackCount, sphere);

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = static_cast<uint32_t> (sizeof(Geometry::VertexData) * sphere.mVertices.size());
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &sphere.mVertices[0];

        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mSphereBufferInfo->mVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        // Set index count
        mSphereBufferInfo->mIndexCount = static_cast<uint32_t> (sphere.mIndices.size());

        // Create and fill index buffer
        D3D11_BUFFER_DESC ibd;
        ibd.Usage = D3D11_USAGE_IMMUTABLE;
        ibd.ByteWidth = sizeof(uint32_t) * mSphereBufferInfo->mIndexCount;
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;
        ibd.StructureByteStride = 0;
        ibd.MiscFlags = 0;

        initData.pSysMem = &sphere.mIndices[0];

        result = device->CreateBuffer(&ibd, &initData, &mSphereBufferInfo->mIndexBuffer);
        DebugUtils::DxErrorChecker(result);
    }

    void GeometryBuffersManager::buildCylinderBuffers(ID3D11Device* device)
    {
        assert(device);

        mCylinderBufferInfo = new IndexedBufferInfo();

        // Set base vertex location
        mCylinderBufferInfo->mBaseVertexLocation = 0;

        // Fill vertices to insert in the vertex buffer
        const float bottomRadius = 10.0f; 
        const float topRadius = 10.0f; 
        const float height = 50.0f;
        const uint32_t sliceCount = 20;
        const uint32_t stackCount = 10;
        Geometry::MeshData cylinder;
        Geometry::GeometryGenerator::createCylinder(bottomRadius, topRadius, height, sliceCount, stackCount, cylinder);

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = static_cast<uint32_t> (sizeof(Geometry::VertexData) * cylinder.mVertices.size());
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &cylinder.mVertices[0];

        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mCylinderBufferInfo->mVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        // Set index count
        mCylinderBufferInfo->mIndexCount = static_cast<uint32_t> (cylinder.mIndices.size());

        // Create and fill index buffer
        D3D11_BUFFER_DESC ibd;
        ibd.Usage = D3D11_USAGE_IMMUTABLE;
        ibd.ByteWidth = sizeof(uint32_t) * mCylinderBufferInfo->mIndexCount;
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;
        ibd.StructureByteStride = 0;
        ibd.MiscFlags = 0;

        initData.pSysMem = &cylinder.mIndices[0];

        result = device->CreateBuffer(&ibd, &initData, &mCylinderBufferInfo->mIndexBuffer);
        DebugUtils::DxErrorChecker(result);
    }

    void GeometryBuffersManager::buildFloorBuffers(ID3D11Device * const device)
    {
        assert(device);

        // Create IndexedBufferInfo for floor
        mFloorBufferInfo = new IndexedBufferInfo();

        // 
        // Calculate vertices and indices
        // Cache vertex offset, index count and offset
        //
        Geometry::MeshData grid;
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
        vertexBufferDesc.ByteWidth = sizeof(Geometry::VertexData) * totalVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &grid.mVertices[0];
        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mFloorBufferInfo->mVertexBuffer);
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
        result = device->CreateBuffer(&indexBufferDesc, &initData, &mFloorBufferInfo->mIndexBuffer);
        DebugUtils::DxErrorChecker(result);
    }
}