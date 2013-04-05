#include "BlendingApp.h"
 
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

namespace Framework
{
    void BlendingApp::updateScene(const float dt)
    {
        // Convert Spherical to Cartesian coordinates.
        const float x = mRadius * sinf(mPhi) * cosf(mTheta);
        const float z = mRadius * sinf(mPhi) * sinf(mTheta);
        const float y = mRadius * cosf(mPhi);

        // Update eye position.        	
        mEyePositionW = DirectX::XMFLOAT3(x, y, z);

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
        if ((TimerUtils::inGameTime(mTimer) - baseTime) >= 0.25f)
        {
            baseTime += 0.25f;

            const uint32_t randomRowIndex = 5 + rand() % 190;
            const uint32_t randomColumnIndex = 5 + rand() % 190;
            const float randomMagnitude = MathHelper::randomFloat(1.0f, 2.0f);

            mWaves.disturb(randomRowIndex, randomColumnIndex, randomMagnitude);
        }

        mWaves.update(dt);

        //
        // Update the wave vertex buffer with the new solution.
        //
        D3D11_MAPPED_SUBRESOURCE mappedData;
        const HRESULT result = mImmediateContext->Map(mWavesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        DxErrorChecker(result);

        Geometry::Vertex* vertex = reinterpret_cast<Geometry::Vertex*> (mappedData.pData);
        for(uint32_t i = 0; i < mWaves.vertices(); ++i)
        {
            vertex[i].mPosition = mWaves[i];
            vertex[i].mNormal = mWaves.normal(i);

            // Derive tex-coords in [0,1] from position.
            vertex[i].mTexCoord.x = 0.5f + mWaves[i].x / mWaves.width();
            vertex[i].mTexCoord.y = 0.5f - mWaves[i].z / mWaves.depth();
        }

        mImmediateContext->Unmap(mWavesVertexBuffer, 0);

        //
        // Animate the lights.
        //

        // Circle light over the land surface.
        mPointLight.mPosition.x = 70.0f * cosf(0.2f * TimerUtils::inGameTime(mTimer));
        mPointLight.mPosition.z = 70.0f * sinf(0.2f * TimerUtils::inGameTime(mTimer));
        mPointLight.mPosition.y = MathHelper::computeMax(height(mPointLight.mPosition.x, 
            mPointLight.mPosition.z), -3.0f) + 10.0f;

        // The spotlight takes on the camera position and is aimed in the
        // same direction the camera is looking.  In this way, it looks
        // like we are holding a flashlight.
        mSpotLight.mPosition = mEyePositionW;
        DirectX::XMVECTOR substraction = DirectX::XMVectorSubtract(target, pos);
        DirectX::XMStoreFloat3(&mSpotLight.mDirection, DirectX::XMVector3Normalize(substraction));

        //
        // Animate water texture coordinates.
        //

        // Tile water texture.
        DirectX::XMMATRIX wavesScale = DirectX::XMMatrixScaling(5.0f, 5.0f, 0.0f);

        // Translate texture over time.
        mWaterTexOffset.y += 0.05f * dt;
        mWaterTexOffset.x += 0.1f * dt;	
        DirectX::XMMATRIX wavesOffset = DirectX::XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

        // Combine scale and translation.
        DirectX::XMStoreFloat4x4(&mWaterTexTransform, wavesScale * wavesOffset);

        //
        // Set pixel shader according to user input.
        //
        if (GetAsyncKeyState('1') & 0x8000)
        {
            mImmediateContext->PSSetShader(mLightingPixelShader, nullptr, 0); 
            mClearScreenColor = DirectX::Colors::LightSkyBlue;
        }

        else if(GetAsyncKeyState('2') & 0x8000)
        {
            mImmediateContext->PSSetShader(mLightingTexturingPixelShader, nullptr, 0); 
            mClearScreenColor = DirectX::Colors::LightSkyBlue;
        }

        else if(GetAsyncKeyState('3') & 0x8000)
        {
            mImmediateContext->PSSetShader(mLightingTexturingFogPixelShader, nullptr, 0); 
            mClearScreenColor = DirectX::Colors::Silver;
        }
    }

    void BlendingApp::drawScene()
    {
        // Update states
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&mClearScreenColor));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        mImmediateContext->IASetInputLayout(mInputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mImmediateContext->VSSetSamplers(0, 1, &mSamplerState);
        mImmediateContext->PSSetSamplers(0, 1, &mSamplerState);

        // Blend factors to be used in blending
        float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

        // Set global index buffer.
        const uint32_t stride = sizeof(Geometry::Vertex);
        const uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        
        // Set constant buffers
        ID3D11Buffer* vertexShaderConstantBuffers[] = { &mPerFrameBuffer.buffer(), &mPerObjectBuffer.buffer() };
        ID3D11Buffer* pixelShaderConstantBuffers[] = { &mPerFrameBuffer.buffer(), &mPerObjectBuffer.buffer(), &mImmutableBuffer.buffer() };
        mImmediateContext->VSSetConstantBuffers(0, 2, vertexShaderConstantBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 3, pixelShaderConstantBuffers);

        // Compute view * projection matrix
        const DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&mView);
        const DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&mProjection);
        const DirectX::XMMATRIX viewProjection = view * projection;

        // Update per frame constant buffers.
        mPerFrameBuffer.mData.mDirectionalLight = mDirectionalLight;
        mPerFrameBuffer.mData.mPointLight = mPointLight;
        mPerFrameBuffer.mData.mSpotLight = mSpotLight;
        mPerFrameBuffer.mData.mEyePositionW = mEyePositionW;
        mPerFrameBuffer.applyChanges(*mImmediateContext);

        //////////////////////////////////////////////////////////////////////////
        // Draw land
        //////////////////////////////////////////////////////////////////////////

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &mLandVertexBuffer, &stride, &offset);
        
        //
        // Update per object constant buffer for land
        //
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mLandWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorld,
            DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjection,
            DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldInverseTranspose,
            DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mGrassTexTransform);
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mTexTransform,
            DirectX::XMMatrixTranspose(texTransform));

        // Set land material and textures
        mPerObjectBuffer.mData.mMaterial = mLandMaterial;
        mImmediateContext->PSSetShaderResources(0, 1, &mGrassTextureSRV);

        // Apply buffer changes and draw.
        mPerObjectBuffer.applyChanges(*mImmediateContext);   
        mImmediateContext->DrawIndexed(mLandIndexCount, mLandIndexOffset, 0);

        //////////////////////////////////////////////////////////////////////////
        // Draw waves
        //////////////////////////////////////////////////////////////////////////

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &mWavesVertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer for waves
        //
        world = DirectX::XMLoadFloat4x4(&mWavesWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorld,
            DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldViewProjection,
            DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mWorldInverseTranspose,
            DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        texTransform = DirectX::XMLoadFloat4x4(&mWaterTexTransform);
        DirectX::XMStoreFloat4x4(&mPerObjectBuffer.mData.mTexTransform,
            DirectX::XMMatrixTranspose(texTransform));

        // Set waves material and textures
        mPerObjectBuffer.mData.mMaterial = mWavesMaterial;
        mImmediateContext->PSSetShaderResources(0, 1, &mWaterTextureSRV);

        // Apply buffer changes and draw.
        mPerObjectBuffer.applyChanges(*mImmediateContext);
        mImmediateContext->OMSetBlendState(mTransparentBS, blendFactor, 0xffffffff);
        mImmediateContext->DrawIndexed(mWavesIndexCount, mWavesIndexOffset, 0);

        // Restore default blend state
        mImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    void BlendingApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
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
            mPhi = MathHelper::clamp(mPhi, 0.1f, DirectX::XM_PI - 0.1f);
        }
        else if( (btnState & MK_RBUTTON) != 0 )
        {
            // Make each pixel correspond to 0.2 unit in the scene.
            const float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
            const float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

            // Update the camera radius based on input.
            mRadius += dx - dy;

            // Restrict the radius.
            mRadius = MathHelper::clamp(mRadius, 50.0f, 500.0f);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void BlendingApp::buildGeometryBuffers()
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
        // each vertex.
        //
        // Pack the vertices of all the meshes into one vertex buffer.
        //
        std::vector<Geometry::Vertex> vertices(totalLandVertexCount);
        {
            DirectX::XMFLOAT3 currentVertexPosition;
            for(size_t i = 0; i < grid.mVertices.size(); ++i)
            {
                currentVertexPosition = grid.mVertices[i].mPosition;
                const float y = height(currentVertexPosition.x, currentVertexPosition.z);
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
        vertexBufferDesc.ByteWidth = sizeof(Geometry::Vertex) * totalLandVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &vertices[0];
        HRESULT result = mDevice->CreateBuffer(&vertexBufferDesc, &initData, &mLandVertexBuffer);
        DxErrorChecker(result);

        // Create waves vertex buffer
        // Note that we allocate space only, as
        // we will be updating the data every time step of the simulation.
        vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::Vertex) * mWaves.vertices();
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vertexBufferDesc.MiscFlags = 0;
        result = mDevice->CreateBuffer(&vertexBufferDesc, 0, &mWavesVertexBuffer);
        DxErrorChecker(result);


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

    void BlendingApp::buildShaders()
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
        DxErrorChecker(result);
        
        fin.open("HLSL/LightingPixelShader.cso", std::ios::binary); 
        fin.seekg(0, std::ios_base::end); 
        size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); 
        compiledShader.resize(size); 
        fin.read(&compiledShader[0], size); 
        fin.close();     

        result = mDevice->CreatePixelShader(&compiledShader[0], size, nullptr, &mLightingPixelShader);
        DxErrorChecker(result);

        fin.open("HLSL/LightingTexturingPixelShader.cso", std::ios::binary); 
        fin.seekg(0, std::ios_base::end); 
        size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); 
        compiledShader.resize(size); 
        fin.read(&compiledShader[0], size); 
        fin.close();     

        result = mDevice->CreatePixelShader(&compiledShader[0], size, nullptr, &mLightingTexturingPixelShader);
        DxErrorChecker(result);

        fin.open("HLSL/LightingTexturingFogPixelShader.cso", std::ios::binary); 
        fin.seekg(0, std::ios_base::end); 
        size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); 
        compiledShader.resize(size); 
        fin.read(&compiledShader[0], size); 
        fin.close();     

        result = mDevice->CreatePixelShader(&compiledShader[0], size, nullptr, &mLightingTexturingFogPixelShader);
        DxErrorChecker(result);

        // Bind vertex shader to the rendering pipeline
        mImmediateContext->VSSetShader(mVertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(mLightingPixelShader, nullptr, 0);
    }
}