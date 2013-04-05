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
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mLandBufferInfo = nullptr;
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mSkyBufferInfo =  nullptr;
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mSphereBufferInfo =  nullptr;

    void GeometryBuffersManager::initAll(ID3D11Device* device)
    {
        assert(device);

        buildLandBuffers(device);
        buildSkyBuffers(device);
        buildSphereBuffers(device);
    }

    void GeometryBuffersManager::destroyAll()
    {
        mLandBufferInfo->mVertexBuffer->Release();
        mLandBufferInfo->mIndexBuffer->Release();
        delete mLandBufferInfo;

        mSkyBufferInfo->mVertexBuffer->Release();
        mSkyBufferInfo->mIndexBuffer->Release();
        delete mSkyBufferInfo;

        mSphereBufferInfo->mVertexBuffer->Release();
        mSphereBufferInfo->mIndexBuffer->Release();
        delete mSphereBufferInfo;
    }

    void GeometryBuffersManager::buildLandBuffers(ID3D11Device* device)
    {
        assert(device);

        // Create IndexedBufferInfo for land
        mLandBufferInfo = new IndexedBufferInfo();

        // 
        // Calculate vertices and indices
        // Cache vertex offset, index count and offset
        //
        Geometry::MeshData grid;
        Geometry::GeometryGenerator::createGrid(400.0f, 400.0f, 100, 100, grid);

        // Cache base vertex location
        mLandBufferInfo->mBaseVertexLocation = 0;

        // Cache the index count
        mLandBufferInfo->mIndexCount = static_cast<uint32_t> (grid.mIndices.size());

        // Cache the starting index
        mLandBufferInfo->mStartIndexLocation = 0; 

        // Compute the total number of vertices
        const uint32_t totalLandVertexCount = static_cast<uint32_t> (grid.mVertices.size());

        // Fill vertices
        std::vector<Geometry::CommonVertex> landVertices(totalLandVertexCount);
        {
            DirectX::XMFLOAT3 currentVertexPosition;
            for(size_t i = 0; i < grid.mVertices.size(); ++i)
            {
                currentVertexPosition = grid.mVertices[i].mPosition;
                const float y = height(currentVertexPosition.x, currentVertexPosition.z);
                currentVertexPosition.y = y;
                landVertices[i].mPosition = currentVertexPosition;
                landVertices[i].mNormal = normal(currentVertexPosition.x, currentVertexPosition.z);
                landVertices[i].mTexCoord = grid.mVertices[i].mTexCoord;
            }      
        } 

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::CommonVertex) * totalLandVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &landVertices[0];
        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mLandBufferInfo->mVertexBuffer);
        DxErrorChecker(result);


        // Fill and Create index buffer
        const uint32_t totalIndexCount = mLandBufferInfo->mIndexCount;
        std::vector<uint32_t> indices;
        indices.reserve(totalIndexCount);
        indices.insert(indices.end(), grid.mIndices.begin(), grid.mIndices.end());

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * totalIndexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &indices[0];
        result = device->CreateBuffer(&indexBufferDesc, &initData, &mLandBufferInfo->mIndexBuffer);
        DxErrorChecker(result);
    }

    void GeometryBuffersManager::buildSkyBuffers(ID3D11Device* device)
    {
        assert(device);

        mSkyBufferInfo = new IndexedBufferInfo();

        // Set base vertex location
        mSkyBufferInfo->mBaseVertexLocation = 0;
        
        // Fill vertices to insert in the vertex buffer
        const float skySphereRadius = 20.0f;
        Geometry::MeshData sphere;
        Geometry::GeometryGenerator::createSphere(skySphereRadius, 30, 30, sphere);

        std::vector<Geometry::SkyVertex> vertices(sphere.mVertices.size());

        for(size_t i = 0; i < sphere.mVertices.size(); ++i)
            vertices[i].mPosition = sphere.mVertices[i].mPosition;

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = static_cast<uint32_t> (sizeof(Geometry::SkyVertex) * vertices.size());
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &vertices[0];

        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mSkyBufferInfo->mVertexBuffer);
        DxErrorChecker(result);

        // Set index count
        mSkyBufferInfo->mIndexCount = static_cast<uint32_t> (sphere.mIndices.size());

        // Create and fill index buffer
        D3D11_BUFFER_DESC ibd;
        ibd.Usage = D3D11_USAGE_IMMUTABLE;
        ibd.ByteWidth = sizeof(uint32_t) * mSkyBufferInfo->mIndexCount;
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;
        ibd.StructureByteStride = 0;
        ibd.MiscFlags = 0;

        std::vector<uint32_t> indices;
        indices.insert(indices.end(), sphere.mIndices.begin(), sphere.mIndices.end());

        initData.pSysMem = &indices[0];

        result = device->CreateBuffer(&ibd, &initData, &mSkyBufferInfo->mIndexBuffer);
        DxErrorChecker(result);
    }

    void GeometryBuffersManager::buildSphereBuffers(ID3D11Device* device)
    {
        assert(device);

        mSphereBufferInfo = new IndexedBufferInfo();

        // Set base vertex location
        mSphereBufferInfo->mBaseVertexLocation = 0;

        // Fill vertices to insert in the vertex buffer
        const float sphereRadius = 70.0f;
        Geometry::MeshData sphere;
        Geometry::GeometryGenerator::createSphere(sphereRadius, 50, 50, sphere);

        std::vector<Geometry::CommonVertex> vertices(sphere.mVertices.size());

        for(size_t i = 0; i < sphere.mVertices.size(); ++i)
        {
            vertices[i].mPosition = sphere.mVertices[i].mPosition;
            vertices[i].mNormal = sphere.mVertices[i].mNormal;
            vertices[i].mTexCoord = sphere.mVertices[i].mTexCoord;
        }

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = static_cast<uint32_t> (sizeof(Geometry::CommonVertex) * vertices.size());
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &vertices[0];

        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mSphereBufferInfo->mVertexBuffer);
        DxErrorChecker(result);

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

        std::vector<uint32_t> indices;
        indices.insert(indices.end(), sphere.mIndices.begin(), sphere.mIndices.end());

        initData.pSysMem = &indices[0];

        result = device->CreateBuffer(&ibd, &initData, &mSphereBufferInfo->mIndexBuffer);
        DxErrorChecker(result);
    }
}