#include "HillApp.h"
 
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
    void HillApp::drawScene()
    {
        // Update state
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::White));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        mImmediateContext->IASetInputLayout(mInputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        const uint32_t stride = sizeof(Geometry::Vertex);
        const uint32_t offset = 0;
        mImmediateContext->IASetVertexBuffers(0, 1, &mGridVertexBuffer, &stride, &offset);
        mImmediateContext->IASetIndexBuffer(mGridIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        ID3D11Buffer* constantBuffer = mPerFrameBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);

        // Update per frame buffer
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mWorld);
        DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&mView);
        DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&mProjection);
        DirectX::XMMATRIX worldViewProjection = world * view * projection;

        DirectX::XMStoreFloat4x4(&mPerFrameBuffer.mData.mWorldViewProjectionTranspose,
            DirectX::XMMatrixTranspose(worldViewProjection));

        mPerFrameBuffer.applyChanges(mImmediateContext);

        // Draw grid        
        mImmediateContext->DrawIndexed(mGridIndexCount, 0, 0);

        const HRESULT result = mSwapChain->Present(0, 0);
        DebugUtils::ErrorChecker(result);
    }

    void HillApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
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
            mPhi = MathUtils::clamp(mPhi, 0.1f, DirectX::XM_PI - 0.1f);
        }
        else if( (btnState & MK_RBUTTON) != 0 )
        {
            // Make each pixel correspond to 0.2 unit in the scene.
            const float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
            const float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

            // Update the camera radius based on input.
            mRadius += dx - dy;

            // Restrict the radius.
            mRadius = MathUtils::clamp(mRadius, 50.0f, 500.0f);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void HillApp::buildGeometryBuffers()
    {
        Geometry::GeometryGenerator::MeshData grid;

        Geometry::GeometryGenerator geoGen;

        geoGen.createGrid(160.0f, 160.0f, 50, 50, grid);

        mGridIndexCount = grid.mIndices.size();

        //
        // Extract the vertex elements we are interested and apply the height function to
        // each vertex.  In addition, color the vertices based on their height so we have
        // sandy looking beaches, grassy low hills, and snow mountain peaks.
        //

        std::vector<Geometry::Vertex> vertices(grid.mVertices.size());
        for(size_t i = 0; i < grid.mVertices.size(); ++i)
        {
            DirectX::XMFLOAT3 p = grid.mVertices[i].mPosition;

            p.y = height(p.x, p.z);

            vertices[i].mPosition   = p;

            // Color the vertex based on its height.
            if( p.y < -10.0f )
            {
                // Sandy beach color.
                vertices[i].mColor = DirectX::XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
            }
            else if( p.y < 5.0f )
            {
                // Light yellow-green.
                vertices[i].mColor = DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
            }
            else if( p.y < 12.0f )
            {
                // Dark yellow-green.
                vertices[i].mColor = DirectX::XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
            }
            else if( p.y < 20.0f )
            {
                // Dark brown.
                vertices[i].mColor = DirectX::XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
            }
            else
            {
                // White snow.
                vertices[i].mColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(Geometry::Vertex) * grid.mVertices.size();
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &vertices[0];
        mDevice->CreateBuffer(&vertexBufferDesc, &initData, &mGridVertexBuffer);

        //
        // Pack the indices of all the meshes into one index buffer.
        //

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * mGridIndexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &grid.mIndices[0];
        mDevice->CreateBuffer(&indexBufferDesc, &initData, &mGridIndexBuffer);
    }

    void HillApp::buildShaders()
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
        DebugUtils::ErrorChecker(result);
        
        fin.open("HLSL/PixelShader.cso", std::ios::binary); 
        fin.seekg(0, std::ios_base::end); 
        size = static_cast<size_t> (fin.tellg()); 
        fin.seekg(0, std::ios_base::beg); 
        compiledShader.resize(size); 
        fin.read(&compiledShader[0], size); 
        fin.close();     

        result = mDevice->CreatePixelShader(&compiledShader[0], size, nullptr, &mPixelShader);
        DebugUtils::ErrorChecker(result);

        // Bind shaders to the rendering pipeline
        mImmediateContext->VSSetShader(mVertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(mPixelShader, nullptr, 0);
    }
}