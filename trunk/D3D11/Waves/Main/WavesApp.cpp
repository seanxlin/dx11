#include "WavesApp.h"
 
#include <fstream>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <vector>

#include "HLSL/Vertex.h"

#include <GeometryGenerator.h>
#include <MathHelper.h>

namespace 
{
    float height(const float x, const float z)
    {
        return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
    }
}

namespace Framework
{

    void WavesApp::updateScene(const float dt)
    {
        // Convert Spherical to Cartesian coordinates.
        const float x = mRadius * sinf(mPhi) * cosf(mTheta);
        const float z = mRadius * sinf(mPhi) * sinf(mTheta);
        const float y = mRadius * cosf(mPhi);

        // Build the view matrix.
        DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
        DirectX::XMVECTOR target = DirectX::XMVectorZero();
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(pos, target, up);
        XMStoreFloat4x4(&mView, viewMatrix);

        //
        // Every quarter second, generate a random wave.
        //
        static float baseTime = 0.0f;
        if ((Utils::TimerUtils::inGameTime(mTimer) - baseTime) >= 0.25f)
        {
            baseTime += 0.25f;

            const uint32_t randomRowIndex = 5 + rand() % 190;
            const uint32_t randomColumnIndex = 5 + rand() % 190;
            const float randomMagnitude = Utils::MathHelper::randomFloat(1.0f, 2.0f);

            mWaves.disturb(randomRowIndex, randomColumnIndex, randomMagnitude);
        }

        mWaves.update(dt);

        //
        // Update the wave vertex buffer with the new solution.
        //
        D3D11_MAPPED_SUBRESOURCE mappedData;
        const HRESULT result = mImmediateContext->Map(mWavesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        DebugUtils::DxErrorChecker(result);

        Geometry::Vertex* vertex = reinterpret_cast<Geometry::Vertex*> (mappedData.pData);
        for(size_t i = 0; i < mWaves.vertices(); ++i)
        {
            vertex[i].mPosition = mWaves[static_cast<uint32_t> (i)];
            vertex[i].mColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        mImmediateContext->Unmap(mWavesVertexBuffer, 0);
    }

    void WavesApp::drawScene()
    {
        // Update states
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::LightSkyBlue));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        mImmediateContext->IASetInputLayout(mInputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        const uint32_t stride = sizeof(Geometry::Vertex);
        const uint32_t offset = 0;
        mImmediateContext->IASetVertexBuffers(0, 1, &mLandVertexBuffer, &stride, &offset);
        mImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        ID3D11Buffer* constantBuffer = &mPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);

        // Compute view * projection matrix
        const DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&mView);
        const DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&mProjection);
        const DirectX::XMMATRIX viewProjection = view * projection;

        //
        // Draw the land, updating first per object buffer
        //
        DirectX::XMMATRIX worldViewProjection = DirectX::XMLoadFloat4x4(&mLandWorld) * viewProjection;
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjectionTranspose,
            DirectX::XMMatrixTranspose(worldViewProjection));

        mPerObjectBuffer.applyChanges(*mImmediateContext);
        mImmediateContext->DrawIndexed(mLandIndexCount, mLandIndexOffset, 0);

        //
        // Draw the waves, updating first per object buffer
        //
        worldViewProjection = DirectX::XMLoadFloat4x4(&mWavesWorld) * viewProjection;
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjectionTranspose,
            DirectX::XMMatrixTranspose(worldViewProjection));

        // We will draw the waves in wireframe mode.
        mImmediateContext->RSSetState(mWireframeRS);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &mWavesVertexBuffer, &stride, &offset);

        mPerObjectBuffer.applyChanges(*mImmediateContext);
        mImmediateContext->DrawIndexed(mWavesIndexCount, mWavesIndexOffset, 0);

        // Reset rasterizer state to default
        mImmediateContext->RSSetState(nullptr);        

        const HRESULT result = mSwapChain->Present(0, 0);
        DebugUtils::DxErrorChecker(result);
    }

    void WavesApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
    {
        if((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
            const float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

            // Update angles based on input to orbit camera around box.
            mTheta += dx;
            mPhi += dy;

            // Restrict the angle mPhi.
            mPhi = Utils::MathHelper::clamp(mPhi, 0.1f, DirectX::XM_PI - 0.1f);
        }
        else if( (btnState & MK_RBUTTON) != 0 )
        {
            // Make each pixel correspond to 0.2 unit in the scene.
            const float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
            const float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

            // Update the camera radius based on input.
            mRadius += dx - dy;

            // Restrict the radius.
            mRadius = Utils::MathHelper::clamp(mRadius, 50.0f, 500.0f);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void WavesApp::buildGeometryBuffers()
    {
        // 
        // Calculate vertices and indices for the land.
        // Cache vertex offset, index count and offset
        //
        Geometry::MeshData grid;
        Geometry::GeometryGenerator::createGrid(160.0f, 160.0f, 50, 50, grid);

        // Cache the index count
        mLandIndexCount = static_cast<uint32_t> (grid.mIndices.size());

        // Cache the starting index
        mLandIndexOffset = 0; 

        // 
        // Calculate vertices and indices for the waves
        // Cache vertex offset, index count and offset
        //

        // Cache the index count
        mWavesIndexCount = mWaves.triangles() * 3;

        // Cache the starting index.
        mWavesIndexOffset = mLandIndexCount;
        
        // Compute the total number of vertices for the land
        const uint32_t totalLandVertexCount = static_cast<uint32_t> (grid.mVertices.size());

        //
        // Extract the vertex elements we are interested and apply the height function to
        // each vertex. In addition, color the vertices based on their height so we have
        // sandy looking beaches, grassy low hills, and snow mountain peaks.
        //
        // Pack the vertices of all the meshes into one vertex buffer.
        //
        std::vector<Geometry::Vertex> vertices(totalLandVertexCount);
        {
            uint32_t k = 0;
            DirectX::XMFLOAT3 currentVertexPosition;
            for(size_t i = 0; i < grid.mVertices.size(); ++i, ++k)
            {
                currentVertexPosition = grid.mVertices[k].mPosition;
                const float y = height(currentVertexPosition.x, currentVertexPosition.z);
                currentVertexPosition.y = y;
                vertices[k].mPosition = currentVertexPosition;

                // Color the vertex based on its height.
                if (y < -10.0f)
                {
                    // Sandy beach color.
                    vertices[k].mColor = DirectX::XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
                }
                else if (y < 5.0f)
                {
                    // Light yellow-green.
                    vertices[k].mColor = DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
                }
                else if (y < 12.0f)
                {
                    // Dark yellow-green.
                    vertices[k].mColor = DirectX::XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
                }
                else if (y < 20.0f)
                {
                    // Dark brown.
                    vertices[k].mColor = DirectX::XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
                }
                else
                {
                    // White snow.
                    vertices[k].mColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
                }
            }      
        } 

        //
        // Create land vertex buffer
        //
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::Vertex) * totalLandVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &vertices[0];
        HRESULT result = mDevice->CreateBuffer(&vertexBufferDesc, &initData, &mLandVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        //
        // Create waves vertex buffer
        // Note that we allocate space only, as
        // we will be updating the data every time step of the simulation.
        vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::Vertex) * mWaves.vertices();
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vertexBufferDesc.MiscFlags = 0;
        result = mDevice->CreateBuffer(&vertexBufferDesc, 0, &mWavesVertexBuffer);
        DebugUtils::DxErrorChecker(result);


        //
        // Create general index buffer (land + waves)
        //

        // Pack the indices of land and waves into one index buffer.
        const uint32_t totalIndexCount = mLandIndexCount + mWavesIndexCount;
        std::vector<uint32_t> indices;
        indices.resize(totalIndexCount);
        indices.insert(indices.begin(), grid.mIndices.begin(), grid.mIndices.end());

        // Iterate over each quad.
        // We begin exactly next the last element copied from grid.
        const uint32_t rows = mWaves.rows();
        const uint32_t columns = mWaves.columns();
        uint32_t k = static_cast<uint32_t> (grid.mIndices.size());
        for(uint32_t i = 0; i < rows - 1; ++i)
        {
            for(uint32_t j = 0; j < columns - 1; ++j)
            {
                indices[k] = i * columns + j;
                indices[k + 1] = i * columns + j + 1;
                indices[k + 2] = (i + 1) * columns + j;

                indices[k + 3] = (i + 1) * columns + j;
                indices[k + 4] = i * columns + j + 1;
                indices[k + 5] = (i + 1) * columns + j + 1;

                k += 6; // next quad
            }
        }

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * totalIndexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &indices[0];
        mDevice->CreateBuffer(&indexBufferDesc, &initData, &mIndexBuffer);
    }

    void WavesApp::buildShaders()
    {
        //
        // Read compiled vertex and pixel shaders and create them.
        //
        std::ifstream fin;
        fin.open("HLSL/VertexShader.cso", std::ios::binary); 
        fin.seekg(0, std::ios_base::end); 
        size_t size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); ;
        std::vector<char> compiledShader(size); 
        fin.read(&compiledShader[0], size); 
        fin.close();

        buildVertexLayout(compiledShader);

        HRESULT result = mDevice->CreateVertexShader(&compiledShader[0], size, nullptr, &mVertexShader);
        DebugUtils::DxErrorChecker(result);
        
        fin.open("HLSL/PixelShader.cso", std::ios::binary); 
        fin.seekg(0, std::ios_base::end); 
        size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); 
        compiledShader.resize(size); 
        fin.read(&compiledShader[0], size); 
        fin.close();     

        result = mDevice->CreatePixelShader(&compiledShader[0], size, nullptr, &mPixelShader);
        DebugUtils::DxErrorChecker(result);

        // Bind shaders to the rendering pipeline
        mImmediateContext->VSSetShader(mVertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(mPixelShader, nullptr, 0);
    }
}