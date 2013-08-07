#include "GeometryBuffersManager.h"

#include <cassert>

#include <GeometryGenerator.h>
#include <DxErrorChecker.h>
#include <MathHelper.h>

namespace 
{
    void buildTerrainBuffers(ID3D11Device& device, GeometryBuffers& geometryBuffers)
    {
        assert(geometryBuffers.mBufferInfo == nullptr);

        // Create IndexedBufferInfo for floor
        geometryBuffers.mBufferInfo = new IndexedBufferInfo();

        // 
        // Calculate vertices and indices
        // Cache vertex offset, index count and offset
        //
        MeshData grid;
        GeometryGenerator::generateGridForInterlockingTiles(512, 512, 32, 32, grid);

        // Cache base vertex location
        geometryBuffers.mBufferInfo->mBaseVertexLocation = 0;

        // Cache the index count
        geometryBuffers.mBufferInfo->mIndexCount = static_cast<uint32_t> (grid.mIndices.size());

        // Cache the starting index
        geometryBuffers.mBufferInfo->mStartIndexLocation = 0; 

        // Compute the total number of vertices
        const uint32_t totalVertexCount = static_cast<uint32_t> (grid.mVertices.size());

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Vertex) * totalVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        std::vector<Vertex> vertices;
        vertices.reserve(totalVertexCount);
        for (size_t vertexIndex = 0; vertexIndex < totalVertexCount; ++vertexIndex)
        {
            vertices.push_back(Vertex(grid.mVertices[vertexIndex].mPosition, grid.mVertices[vertexIndex].mTexCoord));
        }
        initData.pSysMem = &vertices[0];
        HRESULT result = device.CreateBuffer(&vertexBufferDesc, 
                                             &initData, 
                                             &geometryBuffers.mBufferInfo->mVertexBuffer);
        DxErrorChecker(result);

        // Fill and Create index buffer
        const uint32_t totalIndexCount = geometryBuffers.mBufferInfo->mIndexCount;

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * totalIndexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &grid.mIndices[0];
        result = device.CreateBuffer(&indexBufferDesc, 
                                     &initData, 
                                     &geometryBuffers.mBufferInfo->mIndexBuffer);
        DxErrorChecker(result);
    }
}

namespace GeometryBuffersUtils
{   
    void initAll(ID3D11Device& device, GeometryBuffers& geometryBuffers)
    {
        buildTerrainBuffers(device, geometryBuffers);
    }

    void destroyAll(GeometryBuffers& geometryBuffers)
    {
        assert(geometryBuffers.mBufferInfo);

        geometryBuffers.mBufferInfo->mVertexBuffer->Release();
        geometryBuffers.mBufferInfo->mIndexBuffer->Release();
        delete geometryBuffers.mBufferInfo;
    }
}