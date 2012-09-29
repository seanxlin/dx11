#include "BoxApp.h"
 
#include <fstream>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <vector>

#include "HLSL/Vertex.h"

#include <MathHelper.h>

namespace Framework
{
    void BoxApp::drawScene()
    {
        // Update state
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::White));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        mImmediateContext->IASetInputLayout(mInputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        const uint32_t stride = sizeof(Geometry::Vertex);
        const uint32_t offset = 0;
        mImmediateContext->IASetVertexBuffers(0, 1, &mBoxVertexBuffer, &stride, &offset);
        mImmediateContext->IASetIndexBuffer(mBoxIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

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

        // Draw box 
        mImmediateContext->DrawIndexed(36, 0, 0);

        const HRESULT result = mSwapChain->Present(0, 0);
        DebugUtils::DxErrorChecker(result);
    }

    void BoxApp::onMouseMove(WPARAM btnState, const int32_t x, const int32_t y)
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
            // Make each pixel correspond to 0.005 unit in the scene.
            const float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
            const float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

            // Update the camera radius based on input.
            mRadius += dx - dy;

            // Restrict the radius.
            mRadius = Utils::MathHelper::clamp(mRadius, 3.0f, 15.0f);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void BoxApp::buildGeometryBuffers()
    {
        // Create vertex buffer
        Geometry::Vertex vertices[8];
        vertices[0].mPosition = DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f);
        vertices[1].mPosition = DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f);
        vertices[2].mPosition = DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f);
        vertices[3].mPosition = DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f);
        vertices[4].mPosition = DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f);
        vertices[5].mPosition = DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f);
        vertices[6].mPosition = DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f);
        vertices[7].mPosition = DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f);

        DirectX::XMStoreFloat4(&vertices[0].mColor, DirectX::Colors::White);
        DirectX::XMStoreFloat4(&vertices[1].mColor, DirectX::Colors::Black);
        DirectX::XMStoreFloat4(&vertices[2].mColor, DirectX::Colors::Red);
        DirectX::XMStoreFloat4(&vertices[3].mColor, DirectX::Colors::Green);
        DirectX::XMStoreFloat4(&vertices[4].mColor, DirectX::Colors::Blue);
        DirectX::XMStoreFloat4(&vertices[5].mColor, DirectX::Colors::Yellow);
        DirectX::XMStoreFloat4(&vertices[6].mColor, DirectX::Colors::Cyan);
        DirectX::XMStoreFloat4(&vertices[7].mColor, DirectX::Colors::Magenta);

        D3D11_BUFFER_DESC bufferDescription;
        bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDescription.ByteWidth = sizeof(Geometry::Vertex) * 8;
        bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDescription.CPUAccessFlags = 0;
        bufferDescription.MiscFlags = 0;
        bufferDescription.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = vertices;
        HRESULT result = mDevice->CreateBuffer(&bufferDescription, &initData, &mBoxVertexBuffer);
        DebugUtils::DxErrorChecker(result);

        // Create the index buffer
        uint16_t indices[] = {
            // front face
            0, 1, 2,
            0, 2, 3,

            // back face
            4, 6, 5,
            4, 7, 6,

            // left face
            4, 5, 1,
            4, 1, 0,

            // right face
            3, 2, 6,
            3, 6, 7,

            // top face
            1, 5, 6,
            1, 6, 2,

            // bottom face
            4, 0, 3, 
            4, 3, 7
        };

        bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDescription.ByteWidth = sizeof(uint16_t) * 36;
        bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDescription.CPUAccessFlags = 0;
        bufferDescription.MiscFlags = 0;
        bufferDescription.StructureByteStride = 0;

        initData.pSysMem = indices;
        result = mDevice->CreateBuffer(&bufferDescription, &initData, &mBoxIndexBuffer);
        DebugUtils::DxErrorChecker(result);
    }

    void BoxApp::buildShaders()
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