#include "BlurApp.h"
 
#include <DirectXColors.h>
#include <DirectXMath.h>

#include "HLSL/Vertex.h"

#include <DxErrorChecker.h>
#include <MathHelper.h>

namespace Framework
{
    void BlurApp::updateScene(const float dt)
    {
        //
        // Control the camera.
        //
        if (GetAsyncKeyState('W') & 0x8000)
            Utils::CameraUtils::walk(50.0f * dt, mCamera);

        if (GetAsyncKeyState('S') & 0x8000)
            Utils::CameraUtils::walk(-50.0f * dt, mCamera);

        if (GetAsyncKeyState('A') & 0x8000)
            Utils::CameraUtils::strafe(-50.0f * dt, mCamera);

        if (GetAsyncKeyState('D') & 0x8000)
            Utils::CameraUtils::strafe(50.0f * dt, mCamera);
    }

    void BlurApp::drawScene()
    {
        // Render to our offscreen texture.  Note that we can use the same depth/stencil buffer
        // we normally use since our offscreen texture matches the dimensions.  
        ID3D11RenderTargetView* renderTargets = Managers::ResourcesManager::mOffscreenRTV;
        mImmediateContext->OMSetRenderTargets(1, &renderTargets, mDepthStencilView);

        // Update states
        mImmediateContext->ClearRenderTargetView(Managers::ResourcesManager::mOffscreenRTV, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        Utils::CameraUtils::updateViewMatrix(mCamera);

        //
        // Draw the scene to the offscreen texture
        //

        drawLand();

        //
        // Restore the back buffer.  The offscreen render target will serve as an input into
        // the compute shader for blurring, so we must unbind it from the OM stage before we
        // can use it as an input into the compute shader.
        //
        renderTargets = mRenderTargetView;
        mImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

        mBlurFilter.blurInPlace(mImmediateContext, Managers::ResourcesManager::mOffscreenSRV, Managers::ResourcesManager::mOffscreenUAV, 5);

        //
        // Draw fullscreen quad with texture of blurred scene on it.
        //
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        drawScreenQuad();

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DebugUtils::DxErrorChecker(result);
    }

    void BlurApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
    {
        if ((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.35f * static_cast<float>(x - mLastMousePos.x));
            const float dy = DirectX::XMConvertToRadians(0.35f * static_cast<float>(y - mLastMousePos.y));

            Utils::CameraUtils::pitch(dy, mCamera);
            Utils::CameraUtils::rotateY(dx, mCamera);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void BlurApp::drawLand()
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = Utils::CameraUtils::computeViewProjectionMatrix(mCamera);

        // Update per frame constant buffers for land and billboards
        mLandPerFrameBuffer.mData.mDirectionalLight = mDirectionalLight;
        mLandPerFrameBuffer.mData.mSpotLight = mSpotLight;
        mLandPerFrameBuffer.mData.mEyePositionW = mCamera.mPosition;
        mLandPerFrameBuffer.applyChanges(*mImmediateContext);

        // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(Managers::ShadersManager::mCommonIL);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set shaders
        mImmediateContext->VSSetShader(Managers::ShadersManager::mLandVS, nullptr, 0);
        mImmediateContext->PSSetShader(Managers::ShadersManager::mLandPS, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mLinearSS);

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { &mLandPerFrameBuffer.buffer(), &mLandPerObjectBuffer.buffer() };
        ID3D11Buffer* vertexShaderBuffers = &mLandPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        // Get needed info about geometry buffers.
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mLandBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mLandBufferInfo->mIndexBuffer;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mLandBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mLandBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mLandBufferInfo->mIndexCount;

        // Update index buffer
        uint32_t stride = sizeof(Geometry::CommonVertex);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer for land
        //
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mLandWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mLandPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mLandPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mLandPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mSandTexTransform);
        DirectX::XMStoreFloat4x4(&mLandPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));

        // Set land material and textures
        mLandPerObjectBuffer.mData.mMaterial = mSandMaterial;
        mImmediateContext->PSSetShaderResources(0, 1, &Managers::ResourcesManager::mSandSRV);

        // Apply buffer changes and draw.
        mLandPerObjectBuffer.applyChanges(*mImmediateContext);
        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void BlurApp::drawScreenQuad()
    {
        // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(Managers::ShadersManager::mScreenQuadIL);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set shaders
        mImmediateContext->VSSetShader(Managers::ShadersManager::mScreenQuadVS, nullptr, 0);
        mImmediateContext->PSSetShader(Managers::ShadersManager::mScreenQuadPS, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mLinearSS);

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers = &mScreenQuadVSPerFrameBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &pixelShaderBuffers);

        // Get needed info about geometry buffers.
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mScreenQuadBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mScreenQuadBufferInfo->mIndexBuffer;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mScreenQuadBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mScreenQuadBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mScreenQuadBufferInfo->mIndexCount;

        // Update index buffer
        uint32_t stride = sizeof(Geometry::CommonVertex);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        // Update pixel shader per frame buffer 
        DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mScreenQuadVSPerFrameBuffer.mData.mWorldInverseTranspose, identity);
        DirectX::XMStoreFloat4x4(&mScreenQuadVSPerFrameBuffer.mData.mWorldViewProjection, identity);
        DirectX::XMStoreFloat4x4(&mScreenQuadVSPerFrameBuffer.mData.mTexTransform, identity);
        mScreenQuadVSPerFrameBuffer.applyChanges(*mImmediateContext);

        // Set texture
        //ID3D11ShaderResourceView* blurredOuputSRV = mBlurFilter.blurredOutput();
        ID3D11ShaderResourceView* blurredOuputSRV = mBlurFilter.blurredOutput();
        mImmediateContext->PSSetShaderResources(0, 1, &blurredOuputSRV);

        // Apply buffer changes and draw.        
        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);        
    }

    void BlurApp::buildOffscreenViews()
    {
        // We call this function everytime the window is resized so that the render target is a quarter
        // the client area dimensions.  So Release the previous views before we create new ones.

        if (Managers::ResourcesManager::mOffscreenSRV) 
            Managers::ResourcesManager::mOffscreenSRV->Release();

        if (Managers::ResourcesManager::mOffscreenRTV) 
            Managers::ResourcesManager::mOffscreenRTV->Release();

        if (Managers::ResourcesManager::mOffscreenUAV)
            Managers::ResourcesManager::mOffscreenUAV->Release();

        D3D11_TEXTURE2D_DESC texture2DDesc;
        texture2DDesc.Width = mClientWidth;
        texture2DDesc.Height = mClientHeight;
        texture2DDesc.MipLevels = 1;
        texture2DDesc.ArraySize = 1;
        texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture2DDesc.SampleDesc.Count = 1;  
        texture2DDesc.SampleDesc.Quality = 0;  
        texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
        texture2DDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        texture2DDesc.CPUAccessFlags = 0; 
        texture2DDesc.MiscFlags = 0;

        ID3D11Texture2D* offscreenTex = nullptr;
        HRESULT result = mDevice->CreateTexture2D(&texture2DDesc, nullptr, &offscreenTex);
        DebugUtils::DxErrorChecker(result);

        // Null description means to create a view to all mipmap levels using 
        // the format the texture was created with.
        result = mDevice->CreateShaderResourceView(offscreenTex, nullptr, &Managers::ResourcesManager::mOffscreenSRV);
        DebugUtils::DxErrorChecker(result);

        result = mDevice->CreateRenderTargetView(offscreenTex, nullptr, &Managers::ResourcesManager::mOffscreenRTV);
        DebugUtils::DxErrorChecker(result);

        result = mDevice->CreateUnorderedAccessView(offscreenTex, nullptr, &Managers::ResourcesManager::mOffscreenUAV);
        DebugUtils::DxErrorChecker(result);

        // View saves a reference to the texture so we can release our reference.
        offscreenTex->Release();
    }
}