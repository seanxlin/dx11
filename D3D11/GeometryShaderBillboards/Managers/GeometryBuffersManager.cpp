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
}

namespace Managers
{
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mLandBufferInfo = nullptr;
    GeometryBuffersManager::NonIndexedBufferInfo* GeometryBuffersManager::mBillboardsBufferInfo = nullptr;

    void GeometryBuffersManager::initAll(ID3D11Device* device)
    {
        assert(device);

        buildLandBuffers(device);
        buildBillboardsBuffers(device);
    }

    void GeometryBuffersManager::destroyAll()
    {
        delete mLandBufferInfo;
        delete mBillboardsBufferInfo;
    }

    void GeometryBuffersManager::buildLandBuffers(ID3D11Device* device)
    {
        // Create IndexedBufferInfo for land
        mLandBufferInfo = new IndexedBufferInfo();

        // 
        // Calculate vertices and indices for the land.
        // Cache vertex offset, index count and offset
        //
        Geometry::GeometryGenerator::MeshData grid;
        Geometry::GeometryGenerator::createGrid(200.0f, 200.0f, 50, 50, grid);

        // Cache base vertex location
        mLandBufferInfo->mBaseVertexLocation = 0;

        // Cache the index count
        mLandBufferInfo->mIndexCount = static_cast<uint32_t> (grid.mIndices.size());

        // Cache the starting index
        mLandBufferInfo->mStartIndexLocation = 0; 

        // Compute the total number of vertices for the land
        const uint32_t totalLandVertexCount = static_cast<uint32_t> (grid.mVertices.size());

        //
        // Extract the vertex elements we are interested and apply the height function to
        // each vertex.
        //
        // Pack the vertices of all the meshes into one vertex buffer.
        //
        std::vector<Geometry::LandVertex> vertices(totalLandVertexCount);
        {
            DirectX::XMFLOAT3 currentVertexPosition;
            for(size_t i = 0; i < grid.mVertices.size(); ++i)
            {
                currentVertexPosition = grid.mVertices[i].mPosition;
                const float y = Utils::MathHelper::height(currentVertexPosition.x, currentVertexPosition.z);
                currentVertexPosition.y = y;
                vertices[i].mPosition = currentVertexPosition;
                vertices[i].mNormal = normal(currentVertexPosition.x, currentVertexPosition.z);
                vertices[i].mTexCoord = grid.mVertices[i].mTexCoord;
            }      
        } 

        //
        // Create land vertex buffer
        //
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::LandVertex) * totalLandVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &vertices[0];
        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mLandBufferInfo->mVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        //
        // Create land index buffer
        //

        // Pack the indices of land into one index buffer.
        const uint32_t totalIndexCount = mLandBufferInfo->mIndexCount;
        std::vector<uint32_t> indices;
        indices.resize(totalIndexCount);
        indices.insert(indices.begin(), grid.mIndices.begin(), grid.mIndices.end());

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * totalIndexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &indices[0];
        result = device->CreateBuffer(&indexBufferDesc, &initData, &mLandBufferInfo->mIndexBuffer);
        DebugUtils::DxErrorChecker(result);
    }

    void GeometryBuffersManager::buildBillboardsBuffers(ID3D11Device* device)
    {
        // Create NonIndexedBufferInfo for billboards
        mBillboardsBufferInfo = new NonIndexedBufferInfo();

        const uint32_t treeCount = 490;
        mBillboardsBufferInfo->mBaseVertexLocation = 0;
        mBillboardsBufferInfo->mVertexCount = treeCount;
        Geometry::BillboardVertex vertices[treeCount];

        for (uint32_t i = 0; i < treeCount; ++i)
        {
            const float x = Utils::MathHelper::randomFloat(-100.0f, 100.0f);
            const float z = Utils::MathHelper::randomFloat(-100.0f, 100.0f);
            
            // Move tree slightly above land height.
            const float y = Utils::MathHelper::height(x, z) + 10.0f;
            
            vertices[i].mPosition = DirectX::XMFLOAT3(x,y,z);
            vertices[i].mSize = DirectX::XMFLOAT2(24.0f, 24.0f);
        }

        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::BillboardVertex) * treeCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = vertices;
        const HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, 
            &mBillboardsBufferInfo->mVertexBuffer);

        DebugUtils::DxErrorChecker(result);
    }
}