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
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mScreenQuadBufferInfo = nullptr;

    void GeometryBuffersManager::initAll(ID3D11Device* device)
    {
        assert(device);

        buildBuffers(device);
    }

    void GeometryBuffersManager::destroyAll()
    {
        mLandBufferInfo->mVertexBuffer->Release();
        mLandBufferInfo->mIndexBuffer->Release();

        mScreenQuadBufferInfo->mVertexBuffer->Release();

        delete mLandBufferInfo;
        delete mScreenQuadBufferInfo;
    }

    void GeometryBuffersManager::buildBuffers(ID3D11Device* device)
    {
        // Create IndexedBufferInfo for land and screen quad
        mLandBufferInfo = new IndexedBufferInfo();
        mScreenQuadBufferInfo = new IndexedBufferInfo();

        // 
        // Calculate vertices and indices
        // Cache vertex offset, index count and offset
        //
        Geometry::GeometryGenerator::MeshData grid;
        Geometry::GeometryGenerator::createGrid(200.0f, 200.0f, 50, 50, grid);

        Geometry::GeometryGenerator::MeshData quad;
        Geometry::GeometryGenerator::createFullscreenQuad(quad);

        // Cache base vertex location
        mLandBufferInfo->mBaseVertexLocation = 0;
        mScreenQuadBufferInfo->mBaseVertexLocation = 0;

        // Cache the index count
        mLandBufferInfo->mIndexCount = static_cast<uint32_t> (grid.mIndices.size());
        mScreenQuadBufferInfo->mIndexCount = static_cast<uint32_t> (quad.mIndices.size());

        // Cache the starting index
        mLandBufferInfo->mStartIndexLocation = 0; 
        mScreenQuadBufferInfo->mStartIndexLocation = mLandBufferInfo->mIndexCount;

        // Compute the total number of vertices
        const uint32_t totalLandVertexCount = static_cast<uint32_t> (grid.mVertices.size());
        const uint32_t totalScreenQuadVertexCount = static_cast<uint32_t> (quad.mVertices.size());;

        //
        // Extract the vertex elements we are interested and apply the height function to
        // each vertex.
        //
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

        // Build screen quad vertices
        std::vector<Geometry::CommonVertex> screenQuadVertices(totalScreenQuadVertexCount);
        for(size_t i = 0; i < totalScreenQuadVertexCount; ++i)
        {
            screenQuadVertices[i].mPosition = quad.mVertices[i].mPosition;
            screenQuadVertices[i].mNormal = quad.mVertices[i].mNormal;
            screenQuadVertices[i].mTexCoord = quad.mVertices[i].mTexCoord;
        }

        //
        // Create vertex buffers
        //
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::CommonVertex) * totalLandVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &landVertices[0];
        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mLandBufferInfo->mVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::CommonVertex) * totalScreenQuadVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &screenQuadVertices[0];
        result = device->CreateBuffer(&vertexBufferDesc, &initData, &mScreenQuadBufferInfo->mVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        //
        // Create index buffer
        //

        // Pack the indices into one index buffer.
        const uint32_t totalIndexCount = mLandBufferInfo->mIndexCount + mScreenQuadBufferInfo->mIndexCount;
        std::vector<uint32_t> indices;
        indices.reserve(totalIndexCount);
        indices.insert(indices.end(), grid.mIndices.begin(), grid.mIndices.end());
        indices.insert(indices.end(), quad.mIndices.begin(), quad.mIndices.end());

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * totalIndexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &indices[0];
        result = device->CreateBuffer(&indexBufferDesc, &initData, &mLandBufferInfo->mIndexBuffer);
        mScreenQuadBufferInfo->mIndexBuffer = mLandBufferInfo->mIndexBuffer;
        DebugUtils::DxErrorChecker(result);
    }
}