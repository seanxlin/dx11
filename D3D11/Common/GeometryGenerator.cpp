#include "GeometryGenerator.h"

#include <DirectXMath.h>

#include <MathHelper.h>

namespace
{
    void subdivide(MeshData& meshData)
    {
        // Save a copy of the input geometry.
        MeshData inputCopy = meshData;

        const uint32_t numTriangles = static_cast<uint32_t> (inputCopy.mIndices.size() / 3);

        meshData.mVertices.clear();
        meshData.mVertices.reserve(numTriangles * 6);

        meshData.mIndices.clear();
        meshData.mIndices.reserve(numTriangles * 12);

        //       v1
        //       *
        //      / \
        //     /   \
        //  m0*-----*m1
        //   / \   / \
        //  /   \ /   \
        // *-----*-----*
        // v0    m2     v2

        VertexData v0, v1, v2, m0, m1, m2;
        for(uint32_t triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex) {
            v0 = inputCopy.mVertices[ inputCopy.mIndices[triangleIndex * 3 + 0] ];
            v1 = inputCopy.mVertices[ inputCopy.mIndices[triangleIndex * 3 + 1] ];
            v2 = inputCopy.mVertices[ inputCopy.mIndices[triangleIndex * 3 + 2] ];

            // Generate the midpoints.
            // For subdivision, we just care about the position component.  
            // We derive the other vertex components in CreateGeosphere.
            m0.mPosition = DirectX::XMFLOAT3A(
                0.5f * (v0.mPosition.x + v1.mPosition.x),
                0.5f * (v0.mPosition.y + v1.mPosition.y),
                0.5f * (v0.mPosition.z + v1.mPosition.z)
            );

            m1.mPosition = DirectX::XMFLOAT3A(
                0.5f * (v1.mPosition.x + v2.mPosition.x),
                0.5f * (v1.mPosition.y + v2.mPosition.y),
                0.5f * (v1.mPosition.z + v2.mPosition.z)
            );

            m2.mPosition = DirectX::XMFLOAT3A(
                0.5f * (v0.mPosition.x + v2.mPosition.x),
                0.5f * (v0.mPosition.y + v2.mPosition.y),
                0.5f * (v0.mPosition.z + v2.mPosition.z)
            );

            // Add new geometry.
            meshData.mVertices.push_back(v0); // 0
            meshData.mVertices.push_back(v1); // 1
            meshData.mVertices.push_back(v2); // 2
            meshData.mVertices.push_back(m0); // 3
            meshData.mVertices.push_back(m1); // 4
            meshData.mVertices.push_back(m2); // 5

            meshData.mIndices.push_back(triangleIndex * 6 + 0);
            meshData.mIndices.push_back(triangleIndex * 6 + 3);
            meshData.mIndices.push_back(triangleIndex * 6 + 5);

            meshData.mIndices.push_back(triangleIndex * 6 + 3);
            meshData.mIndices.push_back(triangleIndex * 6 + 4);
            meshData.mIndices.push_back(triangleIndex * 6 + 5);

            meshData.mIndices.push_back(triangleIndex * 6 + 5);
            meshData.mIndices.push_back(triangleIndex * 6 + 4);
            meshData.mIndices.push_back(triangleIndex * 6 + 2);

            meshData.mIndices.push_back(triangleIndex * 6 + 3);
            meshData.mIndices.push_back(triangleIndex * 6 + 1);
            meshData.mIndices.push_back(triangleIndex * 6 + 4);
        }
    }


    void buildCylinderTopCap(const float bottomRadius, 
                             const float topRadius, 
                             const float height,
                             const uint32_t sliceCount, 
                             const uint32_t stackCount, 
                             MeshData& meshData)
    {
        const uint32_t baseIndex = static_cast<uint32_t> (meshData.mVertices.size());

        const float y = 0.5f * height;
        const float dTheta = 2.0f * DirectX::XM_PI / sliceCount;

        // Duplicate cap ring vertices because the texture coordinates and normals differ.
        for(size_t sliceIndex = 0; sliceIndex <= sliceCount; ++sliceIndex) {
            const float x = topRadius * cosf(sliceIndex * dTheta);
            const float z = topRadius * sinf(sliceIndex * dTheta);

            // Scale down by the height to try and make top cap texture coord area
            // proportional to base.
            const float u = x / height + 0.5f;
            const float v = z / height + 0.5f;

            const VertexData newVertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v);
            meshData.mVertices.push_back(newVertex);
        }

        // Cap center vertex.
        const VertexData newVertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f);
        meshData.mVertices.push_back(newVertex);

        // Index of center vertex.
        const uint32_t centerIndex = static_cast<uint32_t> (meshData.mVertices.size() - 1);
        for(uint32_t sliceIndex = 0; sliceIndex < sliceCount; ++sliceIndex) {
            meshData.mIndices.push_back(centerIndex);
            meshData.mIndices.push_back(baseIndex + sliceIndex + 1);
            meshData.mIndices.push_back(baseIndex + sliceIndex);
        }
    }

    void buildCylinderBottomCap(const float bottomRadius, 
                                const float topRadius, 
                                const float height,
                                const uint32_t sliceCount, 
                                const uint32_t stackCount, 
                                MeshData& meshData)
    {	 
        // Build bottom cap.
        const uint32_t baseIndex = static_cast<uint32_t> (meshData.mVertices.size());
        const float y = -0.5f * height;

        // vertices of ring
        const float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
        for(uint32_t sliceIndex = 0; sliceIndex <= sliceCount; ++sliceIndex) {
            const float x = bottomRadius * cosf(sliceIndex * dTheta);
            const float z = bottomRadius * sinf(sliceIndex * dTheta);

            // Scale down by the height to try and make top cap texture coord area
            // proportional to base.
            const float u = x / height + 0.5f;
            const float v = z / height + 0.5f;

            const VertexData newVertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v);
            meshData.mVertices.push_back(newVertex);
        }

        // Cap center vertex.
        const VertexData newVertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f);
        meshData.mVertices.push_back(newVertex);

        // Cache the index of center vertex.
        const uint32_t centerIndex = static_cast<uint32_t> (meshData.mVertices.size() - 1);
        for(uint32_t sliceIndex = 0; sliceIndex < sliceCount; ++sliceIndex) {
            meshData.mIndices.push_back(centerIndex);
            meshData.mIndices.push_back(baseIndex + sliceIndex);
            meshData.mIndices.push_back(baseIndex + sliceIndex + 1);
        }
    }
}

namespace GeometryGenerator
{
    void generateBox(const float width, 
                     const float height, 
                     const float depth, 
                     MeshData& meshData)
    {
        // Create the vertices.
        VertexData vertices[24];

        const float halfWidth = 0.5f * width;
        const float halfHeight = 0.5f * height;
        const float halfDepth = 0.5f * depth;

        // Fill in the front face vertex data.
        vertices[0] = VertexData(-halfWidth, -halfHeight, -halfDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        vertices[1] = VertexData(-halfWidth, +halfHeight, -halfDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        vertices[2] = VertexData(+halfWidth, +halfHeight, -halfDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        vertices[3] = VertexData(+halfWidth, -halfHeight, -halfDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

        // Fill in the back face vertex data.
        vertices[4] = VertexData(-halfWidth, -halfHeight, +halfDepth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
        vertices[5] = VertexData(+halfWidth, -halfHeight, +halfDepth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        vertices[6] = VertexData(+halfWidth, +halfHeight, +halfDepth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        vertices[7] = VertexData(-halfWidth, +halfHeight, +halfDepth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

        // Fill in the top face vertex data.
        vertices[8]  = VertexData(-halfWidth, +halfHeight, -halfDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        vertices[9]  = VertexData(-halfWidth, +halfHeight, +halfDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        vertices[10] = VertexData(+halfWidth, +halfHeight, +halfDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        vertices[11] = VertexData(+halfWidth, +halfHeight, -halfDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

        // Fill in the bottom face vertex data.
        vertices[12] = VertexData(-halfWidth, -halfHeight, -halfDepth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
        vertices[13] = VertexData(+halfWidth, -halfHeight, -halfDepth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        vertices[14] = VertexData(+halfWidth, -halfHeight, +halfDepth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        vertices[15] = VertexData(-halfWidth, -halfHeight, +halfDepth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

        // Fill in the left face vertex data.
        vertices[16] = VertexData(-halfWidth, -halfHeight, +halfDepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
        vertices[17] = VertexData(-halfWidth, +halfHeight, +halfDepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
        vertices[18] = VertexData(-halfWidth, +halfHeight, -halfDepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
        vertices[19] = VertexData(-halfWidth, -halfHeight, -halfDepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

        // Fill in the right face vertex data.
        vertices[20] = VertexData(+halfWidth, -halfHeight, -halfDepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        vertices[21] = VertexData(+halfWidth, +halfHeight, -halfDepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        vertices[22] = VertexData(+halfWidth, +halfHeight, +halfDepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
        vertices[23] = VertexData(+halfWidth, -halfHeight, +halfDepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

        meshData.mVertices.assign(&vertices[0], &vertices[24]);

        // Create the indices.
        uint32_t indices[36];

        // Fill in the front face index data
        indices[0] = 0; indices[1] = 1; indices[2] = 2;
        indices[3] = 0; indices[4] = 2; indices[5] = 3;

        // Fill in the back face index data
        indices[6] = 4; indices[7]  = 5; indices[8]  = 6;
        indices[9] = 4; indices[10] = 6; indices[11] = 7;

        // Fill in the top face index data
        indices[12] = 8; indices[13] =  9; indices[14] = 10;
        indices[15] = 8; indices[16] = 10; indices[17] = 11;

        // Fill in the bottom face index data
        indices[18] = 12; indices[19] = 13; indices[20] = 14;
        indices[21] = 12; indices[22] = 14; indices[23] = 15;

        // Fill in the left face index data
        indices[24] = 16; indices[25] = 17; indices[26] = 18;
        indices[27] = 16; indices[28] = 18; indices[29] = 19;

        // Fill in the right face index data
        indices[30] = 20; indices[31] = 21; indices[32] = 22;
        indices[33] = 20; indices[34] = 22; indices[35] = 23;

        meshData.mIndices.assign(&indices[0], &indices[36]);
    }

    void generateSphere(const float radius, 
                        const uint32_t sliceCount, 
                        const uint32_t stackCount, 
                        MeshData& meshData)
    {
        meshData.mVertices.clear();
        meshData.mVertices.reserve(stackCount * (sliceCount - 1));

        meshData.mIndices.clear();
        meshData.mIndices.reserve(sliceCount + ((stackCount - 1) * (sliceCount)) + sliceCount);

        // Compute the vertices stating at the top pole and moving down the stacks.
        // Poles: note that there will be texture coordinate distortion as there is
        // not a unique point on the texture map to assign to the pole when mapping
        // a rectangular texture onto a sphere.
        VertexData topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        VertexData bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

        meshData.mVertices.push_back(topVertex);

        const float phiStep = DirectX::XM_PI / stackCount;
        const float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

        {
            // Compute vertices for each stack ring (do not count the poles as rings).
            DirectX::XMVECTOR tangentU;
            DirectX::XMVECTOR position;
            for(size_t stackIndex = 1; stackIndex <= stackCount - 1; ++stackIndex) {
                const float phi = stackIndex * phiStep;

                // Vertices of ring.
                VertexData vertex;		
                for(size_t sliceIndex = 0; sliceIndex <= sliceCount; ++sliceIndex) {
                    const float theta = sliceIndex * thetaStep;

                    // spherical to cartesian
                    vertex.mPosition.x = radius * sinf(phi) * cosf(theta);
                    vertex.mPosition.y = radius * cosf(phi);
                    vertex.mPosition.z = radius * sinf(phi) * sinf(theta);

                    // Partial derivative of P with respect to theta
                    vertex.mTangentU.x = -radius * sinf(phi) * sinf(theta);
                    vertex.mTangentU.y = 0.0f;
                    vertex.mTangentU.z = +radius * sinf(phi) * cosf(theta);

                    tangentU = DirectX::XMLoadFloat3(&vertex.mTangentU);
                    DirectX::XMStoreFloat3(&vertex.mTangentU, DirectX::XMVector3Normalize(tangentU));

                    position = DirectX::XMLoadFloat3(&vertex.mPosition);
                    DirectX::XMStoreFloat3(&vertex.mNormal, DirectX::XMVector3Normalize(position));

                    vertex.mTexCoord.x = theta / DirectX::XM_2PI;
                    vertex.mTexCoord.y = phi / DirectX::XM_PI;

                    meshData.mVertices.push_back(vertex);
                }
            }
        }

        meshData.mVertices.push_back(bottomVertex);

        // Compute indices for top stack.  The top stack was written first to the vertex buffer
        // and connects the top pole to the first ring.
        for(uint32_t sliceIndex = 1; sliceIndex <= sliceCount; ++sliceIndex) {
            meshData.mIndices.push_back(0);
            meshData.mIndices.push_back(sliceIndex+1);
            meshData.mIndices.push_back(sliceIndex);
        }

        // Compute indices for inner stacks (not connected to poles).
        // Offset the indices to the index of the first vertex in the first ring.
        // This is just skipping the top pole vertex.
        uint32_t baseIndex = 1;
        const uint32_t ringVertexCount = sliceCount + 1;
        for(uint32_t stackIndex = 0; stackIndex < stackCount - 2; ++stackIndex) {
            for(uint32_t sliceIndex = 0; sliceIndex < sliceCount; ++sliceIndex) {
                meshData.mIndices.push_back(baseIndex + stackIndex * ringVertexCount + sliceIndex);
                meshData.mIndices.push_back(baseIndex + stackIndex * ringVertexCount + sliceIndex+1);
                meshData.mIndices.push_back(baseIndex + (stackIndex + 1) * ringVertexCount + sliceIndex);

                meshData.mIndices.push_back(baseIndex + (stackIndex + 1) * ringVertexCount + sliceIndex);
                meshData.mIndices.push_back(baseIndex + stackIndex * ringVertexCount + sliceIndex + 1);
                meshData.mIndices.push_back(baseIndex + (stackIndex + 1) * ringVertexCount + sliceIndex + 1);
            }
        }

        // Compute indices for bottom stack. The bottom stack was written last to the vertex buffer
        // and connects the bottom pole to the bottom ring.
        // South pole vertex was added last.
        const uint32_t southPoleIndex = static_cast<uint32_t> (meshData.mVertices.size() - 1);

        // Offset the indices to the index of the first vertex in the last ring.
        baseIndex = southPoleIndex - ringVertexCount;

        for(uint32_t sliceIndex = 0; sliceIndex < sliceCount; ++sliceIndex) {
            meshData.mIndices.push_back(southPoleIndex);
            meshData.mIndices.push_back(baseIndex + sliceIndex);
            meshData.mIndices.push_back(baseIndex + sliceIndex + 1);
        }
    }

    void generateGeosphere(const float radius, 
                           const uint32_t numSubdivisions, 
                           MeshData& meshData)
    {
        // Approximate a sphere by tessellating an icosahedron.
        const float factor0 = 0.525731f; 
        const float factor1 = 0.850651f;

        const size_t numPositions = 12;
        DirectX::XMFLOAT3A positions[numPositions] = 
        {
            DirectX::XMFLOAT3A(-factor0, 0.0f, factor1), 
            DirectX::XMFLOAT3A(factor0, 0.0f, factor1),  
            DirectX::XMFLOAT3A(-factor0, 0.0f, -factor1), 
            DirectX::XMFLOAT3A(factor0, 0.0f, -factor1),    
            DirectX::XMFLOAT3A(0.0f, factor1, factor0), 
            DirectX::XMFLOAT3A(0.0f, factor1, -factor0), 
            DirectX::XMFLOAT3A(0.0f, -factor1, factor0),  
            DirectX::XMFLOAT3A(0.0f, -factor1, -factor0),    
            DirectX::XMFLOAT3A(factor1, factor0, 0.0f), 
            DirectX::XMFLOAT3A(-factor1, factor0, 0.0f), 
            DirectX::XMFLOAT3A(factor1, -factor0, 0.0f),
            DirectX::XMFLOAT3A(-factor1, -factor0, 0.0f)
        };

        const size_t numIndices = 60;
        uint32_t indices[numIndices] = 
        {
            1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,    
            1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,    
            3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0, 
            10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7 
        };

        meshData.mVertices.resize(numPositions);
        meshData.mIndices.resize(numIndices);

        for(size_t positionIndex = 0; positionIndex < numPositions; ++positionIndex) {
            meshData.mVertices[positionIndex].mPosition = positions[positionIndex];
        }

        for(size_t indicesIndex = 0; indicesIndex < numIndices; ++indicesIndex) {
            meshData.mIndices[indicesIndex] = indices[indicesIndex];
        }

        for(size_t subdivisionIndex = 0; 
                   subdivisionIndex < numSubdivisions; 
                   ++subdivisionIndex) {
            subdivide(meshData);
        }

        {
            // Project vertices onto sphere and scale.
            DirectX::XMVECTOR normal;
            DirectX::XMVECTOR position;
            DirectX::XMVECTOR tangentU;
            DirectX::XMFLOAT3A normalAux;
            const size_t numVertices = meshData.mVertices.size();
            for(size_t vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex) {
                // Project onto unit sphere.
                normal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&meshData.mVertices[vertexIndex].mPosition));
                DirectX::XMStoreFloat3(&meshData.mVertices[vertexIndex].mNormal, normal);

                // Project onto sphere.
                normalAux = DirectX::XMFLOAT3A(meshData.mVertices[vertexIndex].mNormal.x * radius,
                    meshData.mVertices[vertexIndex].mNormal.y * radius,
                    meshData.mVertices[vertexIndex].mNormal.z * radius);
                position = DirectX::XMLoadFloat3A(&normalAux);
                DirectX::XMStoreFloat3(&meshData.mVertices[vertexIndex].mPosition, position);

                // Derive texture coordinates from spherical coordinates.
                const float theta = MathHelper::angle(
                    meshData.mVertices[vertexIndex].mPosition.x, 
                    meshData.mVertices[vertexIndex].mPosition.z);

                const float phi = acosf(meshData.mVertices[vertexIndex].mPosition.y / radius);

                meshData.mVertices[vertexIndex].mTexCoord.x = theta / DirectX::XM_2PI;
                meshData.mVertices[vertexIndex].mTexCoord.y = phi / DirectX::XM_PI;

                // Partial derivative of P with respect to theta
                meshData.mVertices[vertexIndex].mTangentU.x = -radius * sinf(phi) * sinf(theta);
                meshData.mVertices[vertexIndex].mTangentU.y = 0.0f;
                meshData.mVertices[vertexIndex].mTangentU.z = +radius * sinf(phi) * cosf(theta);

                tangentU = DirectX::XMLoadFloat3(&meshData.mVertices[vertexIndex].mTangentU);
                DirectX::XMStoreFloat3(&meshData.mVertices[vertexIndex].mTangentU, 
                                       DirectX::XMVector3Normalize(tangentU));
            }
        }
    }

    void GeometryGenerator::generateCylinder(const float bottomRadius, 
                                             const float topRadius, 
                                             const float height, 
                                             const uint32_t sliceCount, 
                                             const uint32_t stackCount,
                                             MeshData& meshData)
    {
        meshData.mVertices.clear();
        const uint32_t ringCount = stackCount + 1;
        meshData.mVertices.reserve(ringCount * (sliceCount + 1));

        meshData.mIndices.clear();
        meshData.mIndices.reserve(6 * stackCount * sliceCount);

        // Build Stacks.
        const float stackHeight = height / stackCount;

        // Amount to increment radius as we move up each stack level from bottom to top.
        const float radiusStep = (topRadius - bottomRadius) / stackCount;

        // Compute vertices for each stack ring starting at the bottom and moving up.
        {
            VertexData vertex;	
            DirectX::XMVECTOR tangentU;
            DirectX::XMVECTOR biTangentU;
            DirectX::XMVECTOR normal;
            DirectX::XMFLOAT3 bitangent;
            for(size_t ringIndex = 0; ringIndex < ringCount; ++ringIndex) {
                const float y = -0.5f * height + ringIndex * stackHeight;
                const float r = bottomRadius + ringIndex * radiusStep;

                // vertices of ring
                const float dTheta = 2.0f * DirectX::XM_PI / sliceCount;		
                for(size_t sliceIndex = 0; sliceIndex <= sliceCount; ++sliceIndex) {
                    const float c = cosf(sliceIndex * dTheta);
                    const float s = sinf(sliceIndex * dTheta);

                    vertex.mPosition = DirectX::XMFLOAT3(r * c, y, r * s);

                    vertex.mTexCoord.x = static_cast<float> (sliceIndex) / sliceCount;
                    vertex.mTexCoord.y = 1.0f - static_cast<float> (ringIndex) / stackCount;

                    // Cylinder can be parameterized as follows, where we introduce v
                    // parameter that goes in the same direction as the v tex-coord
                    // so that the bitangent goes in the same direction as the v tex-coord.
                    //   Let r0 be the bottom radius and let r1 be the top radius.
                    //   y(v) = h - hv for v in [0,1].
                    //   r(v) = r1 + (r0-r1)v
                    //
                    //   x(t, v) = r(v)*cos(t)
                    //   y(t, v) = h - hv
                    //   z(t, v) = r(v)*sin(t)
                    // 
                    //  dx/dt = -r(v)*sin(t)
                    //  dy/dt = 0
                    //  dz/dt = +r(v)*cos(t)
                    //
                    //  dx/dv = (r0-r1)*cos(t)
                    //  dy/dv = -h
                    //  dz/dv = (r0-r1)*sin(t)

                    // This is unit length.
                    vertex.mTangentU = DirectX::XMFLOAT3(-s, 0.0f, c);

                    const float dr = bottomRadius - topRadius;
                    bitangent = DirectX::XMFLOAT3(dr * c, -height, dr * s);

                    tangentU = DirectX::XMLoadFloat3(&vertex.mTangentU);
                    biTangentU = DirectX::XMLoadFloat3(&bitangent);
                    normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(tangentU, biTangentU));
                    DirectX::XMStoreFloat3(&vertex.mNormal, normal);

                    meshData.mVertices.push_back(vertex);
                }
            }
        }

        // Add one because we duplicate the first and last vertex per ring
        // since the texture coordinates are different.
        const uint32_t ringVertexCount = sliceCount + 1;

        // Compute indices for each stack.
        for(uint32_t stackIndex = 0; stackIndex < stackCount; ++stackIndex) {
            for(uint32_t sliceIndex = 0; sliceIndex < sliceCount; ++sliceIndex) {
                meshData.mIndices.push_back(stackIndex * ringVertexCount + sliceIndex);
                meshData.mIndices.push_back((stackIndex + 1) * ringVertexCount + sliceIndex);
                meshData.mIndices.push_back((stackIndex + 1) * ringVertexCount + sliceIndex + 1);

                meshData.mIndices.push_back(stackIndex * ringVertexCount + sliceIndex);
                meshData.mIndices.push_back((stackIndex + 1) * ringVertexCount + sliceIndex + 1);
                meshData.mIndices.push_back(stackIndex * ringVertexCount + sliceIndex + 1);
            }
        }

        buildCylinderTopCap(bottomRadius, 
                            topRadius, 
                            height, 
                            sliceCount, 
                            stackCount, 
                            meshData);

        buildCylinderBottomCap(bottomRadius, 
                               topRadius, 
                               height, 
                               sliceCount, 
                               stackCount, 
                               meshData);
    }

    void generateGrid(const float width, 
                      const float depth, 
                      const uint32_t numRows, 
                      const uint32_t numColumns, 
                      MeshData& meshData)
    {
        const uint32_t vexterPerColumn = numColumns + 1;
        const uint32_t vexterPerRow = numRows + 1;
        const uint32_t vertexCount = vexterPerRow * vexterPerColumn;
        const uint32_t faceCount   = (vexterPerRow) * (vexterPerColumn) * 2;

        // Create the vertices.
        const float halfWidth = 0.5f * width;
        const float halfDepth = 0.5f * depth;

        const float dx = width / (vexterPerColumn);
        const float dz = depth / (vexterPerRow);

        const float du = 1.0f / (vexterPerColumn);
        const float dv = 1.0f / (vexterPerRow);

        meshData.mVertices.resize(vertexCount);
        for(size_t vertexPerRowIndex = 0; 
                   vertexPerRowIndex < vexterPerRow; 
                   ++vertexPerRowIndex) {
            const float z = halfDepth - vertexPerRowIndex * dz;
            for(size_t vertexPerColumnIndex = 0; 
                       vertexPerColumnIndex < vexterPerColumn; 
                       ++vertexPerColumnIndex) {
                const float x = -halfWidth + vertexPerColumnIndex * dx;

                const size_t vertexIndex = vertexPerRowIndex * vexterPerColumn + vertexPerColumnIndex;
                meshData.mVertices[vertexIndex].mPosition = DirectX::XMFLOAT3(x, 0.0f, z);
                meshData.mVertices[vertexIndex].mNormal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
                meshData.mVertices[vertexIndex].mTangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

                // Stretch texture over grid.
                meshData.mVertices[vertexIndex].mTexCoord.x = vertexPerColumnIndex * du;
                meshData.mVertices[vertexIndex].mTexCoord.y = vertexPerRowIndex * dv;
            }
        }

        // Create the indices.
        meshData.mIndices.resize(faceCount * 3); // 3 indices per face

        // Iterate over each quad and compute indices.
        uint32_t k = 0;
        for(uint32_t vertexPerRowIndex = 0; 
                     vertexPerRowIndex < vexterPerRow - 1; 
                     ++vertexPerRowIndex) {
            for(uint32_t vertexPerColumnIndex = 0; 
                         vertexPerColumnIndex < vexterPerColumn - 1; 
                         ++vertexPerColumnIndex) {
                meshData.mIndices[k] = vertexPerRowIndex * vexterPerColumn + vertexPerColumnIndex;
                meshData.mIndices[k + 1] = vertexPerRowIndex * vexterPerColumn + vertexPerColumnIndex + 1;
                meshData.mIndices[k + 2] = (vertexPerRowIndex + 1) * vexterPerColumn + vertexPerColumnIndex;

                meshData.mIndices[k + 3] = (vertexPerRowIndex + 1) * vexterPerColumn + vertexPerColumnIndex;
                meshData.mIndices[k + 4] = vertexPerRowIndex * vexterPerColumn + vertexPerColumnIndex + 1;
                meshData.mIndices[k + 5] = (vertexPerRowIndex + 1) * vexterPerColumn + vertexPerColumnIndex + 1;

                k += 6; // next quad
            }
        }
    }

    void generateGridForInterlockingTiles(const float width, 
                                          const float depth, 
                                          const uint32_t numRows, 
                                          const uint32_t numColumns, 
                                          MeshData& meshData)
    {
        const uint32_t vexterPerColumn = numColumns + 1;
        const uint32_t vexterPerRow = numRows + 1;
        const uint32_t vertexCount = vexterPerRow * vexterPerColumn;
        const uint32_t faceCount = (vexterPerRow) * (vexterPerColumn) * 2;

        // Create the vertices.
        const float halfWidth = 0.5f * width;
        const float halfDepth = 0.5f * depth;

        const float dx = width / vexterPerColumn;
        const float dz = depth / vexterPerRow;

        const float du = 1.0f / vexterPerColumn;
        const float dv = 1.0f / vexterPerRow;

        meshData.mVertices.resize(vertexCount);
        for(size_t vertexPerRowIndex = 0; 
                   vertexPerRowIndex < vexterPerRow; 
                   ++vertexPerRowIndex) {
            const float zValue = halfDepth - vertexPerRowIndex * dz;
            for(size_t vertexPerColumnIndex = 0; 
                       vertexPerColumnIndex < vexterPerColumn; 
                       ++vertexPerColumnIndex) {
                const float xValue = -halfWidth + vertexPerColumnIndex * dx;

                const size_t vertexIndex = vertexPerRowIndex * vexterPerColumn + vertexPerColumnIndex;
                meshData.mVertices[vertexIndex].mPosition = DirectX::XMFLOAT3(xValue, 0.0f, zValue);
                meshData.mVertices[vertexIndex].mNormal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
                meshData.mVertices[vertexIndex].mTangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

                // Stretch texture over grid.
                meshData.mVertices[vertexIndex].mTexCoord.x = vertexPerColumnIndex * du;
                meshData.mVertices[vertexIndex].mTexCoord.y = vertexPerRowIndex * dv;
            }
        }

        // Create the indices.
        const uint32_t controlPointsPerQuad = 12;
        meshData.mIndices.resize(numRows * numColumns * controlPointsPerQuad);

        // Iterate over each quad and compute indices.
        uint32_t k = 0;
        for(uint32_t rowIndex = 0; rowIndex < numRows; ++rowIndex) {
            for(uint32_t columnIndex = 0; columnIndex < numColumns; ++columnIndex) {
                // 0-3 are the actual quad vertices
                meshData.mIndices[k + 0] = (columnIndex + 0) + (rowIndex + 0) * vexterPerColumn;
                meshData.mIndices[k + 1] = (columnIndex + 1) + (rowIndex + 0) * vexterPerColumn;
                meshData.mIndices[k + 2] = (columnIndex + 0) + (rowIndex + 1) * vexterPerColumn;
                meshData.mIndices[k + 3] = (columnIndex + 1) + (rowIndex + 1) * vexterPerColumn;

                // 4-5 are +z
                meshData.mIndices[k + 4] = MathHelper::clamp<uint32_t>(columnIndex + 0, 0, numColumns) + MathHelper::clamp<uint32_t>(rowIndex + 2, 0, numRows) * vexterPerColumn;
                meshData.mIndices[k + 5] = MathHelper::clamp<uint32_t>(columnIndex + 1, 0, numColumns) + MathHelper::clamp<uint32_t>(rowIndex + 2, 0, numRows) * vexterPerColumn;

                // 6-7 are +x
                meshData.mIndices[k + 6] = MathHelper::clamp<uint32_t>(columnIndex + 2, 0, numColumns) + MathHelper::clamp<uint32_t>(rowIndex + 0, 0, numRows) * vexterPerColumn;
                meshData.mIndices[k + 7] = MathHelper::clamp<uint32_t>(columnIndex + 2, 0, numColumns) + MathHelper::clamp<uint32_t>(rowIndex + 1, 0, numRows) * vexterPerColumn;

                // 8-9 are -z
                meshData.mIndices[k + 8] = MathHelper::clamp<uint32_t>(columnIndex + 0, 0, numColumns) + MathHelper::clamp<uint32_t>(rowIndex - 1, 0, numRows) * vexterPerColumn;
                meshData.mIndices[k + 9] = MathHelper::clamp<uint32_t>(columnIndex + 1, 0, numColumns) + MathHelper::clamp<uint32_t>(rowIndex - 1, 0, numRows) * vexterPerColumn;

                // 10-11 are -x
                meshData.mIndices[k + 10] = MathHelper::clamp<uint32_t>(columnIndex - 1, 0, numColumns) + MathHelper::clamp<uint32_t>(rowIndex + 0, 0, numRows) * vexterPerColumn;
                meshData.mIndices[k + 11] = MathHelper::clamp<uint32_t>(columnIndex - 1, 0, numColumns) + MathHelper::clamp<uint32_t>(rowIndex + 1, 0, numRows) * vexterPerColumn;

                k += controlPointsPerQuad; // 12 control points per quad
            }
        }
    }

    void generateFullscreenQuad(MeshData& meshData)
    {
        meshData.mVertices.resize(4);
        meshData.mIndices.resize(6);

        // Position coordinates specified in NDC space.
        meshData.mVertices[0] = VertexData(
            -1.0f, -1.0f, 0.0f, 
            0.0f, 0.0f, -1.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f
        );

        meshData.mVertices[1] = VertexData(
            -1.0f, +1.0f, 0.0f, 
            0.0f, 0.0f, -1.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f
        );

        meshData.mVertices[2] = VertexData(
            +1.0f, +1.0f, 0.0f, 
            0.0f, 0.0f, -1.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f
        );

        meshData.mVertices[3] = VertexData(
            +1.0f, -1.0f, 0.0f, 
            0.0f, 0.0f, -1.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f
        );

        meshData.mIndices[0] = 0;
        meshData.mIndices[1] = 1;
        meshData.mIndices[2] = 2;

        meshData.mIndices[3] = 0;
        meshData.mIndices[4] = 2;
        meshData.mIndices[5] = 3;
    }
}
