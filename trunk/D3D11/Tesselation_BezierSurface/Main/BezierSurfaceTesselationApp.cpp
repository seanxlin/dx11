#include "BezierSurfaceTesselationApp.h"
 
#include <algorithm>
#include <DirectXColors.h>
#include <DirectXMath.h>

#include "HLSL/Vertex.h"

#include <DxErrorChecker.h>
#include <MathHelper.h>

namespace
{
    const float gMinTessellationFactor = 0.0f;
    const float gMaxTessellationFactor = 64.0f;
    const float gTessellationOffset = 0.0025f;
}

namespace Framework
{
    void BezierSurfaceTesselationApp::updateScene(const float dt)
    {
        //
        // Control the camera.
        //
        if (GetAsyncKeyState('W') & 0x8000)
            CameraUtils::walk(50.0f * dt, mCamera);

        if (GetAsyncKeyState('S') & 0x8000)
            CameraUtils::walk(-50.0f * dt, mCamera);

        if (GetAsyncKeyState('A') & 0x8000)
            CameraUtils::strafe(-50.0f * dt, mCamera);

        if (GetAsyncKeyState('D') & 0x8000)
            CameraUtils::strafe(50.0f * dt, mCamera);

        /*if (GetAsyncKeyState('G') & 0x8000)
            mTesselationFactor = (gMaxTessellationFactor < mTesselationFactor + gTessellationOffset) ? gMaxTessellationFactor : mTesselationFactor + gTessellationOffset;

        if (GetAsyncKeyState('H') & 0x8000) 
            mTesselationFactor = (mTesselationFactor - gTessellationOffset < gMinTessellationFactor) ? gMinTessellationFactor : mTesselationFactor - gTessellationOffset;*/

        if (GetAsyncKeyState('F') & 0x8000) 
            mTesselationFactor = 8.0f;

        if (GetAsyncKeyState('G') & 0x8000) 
            mTesselationFactor = 16.0f;

        if (GetAsyncKeyState('H') & 0x8000) 
            mTesselationFactor = 32.0f;

        if (GetAsyncKeyState('J') & 0x8000) 
            mTesselationFactor = 64.0f;

        if (GetAsyncKeyState('T') & 0x8000) 
            mWireframeMode = true;

        if (GetAsyncKeyState('Y') & 0x8000) 
            mWireframeMode = false;

        mRotationAmmount += 0.25f * dt;
    }

    void BezierSurfaceTesselationApp::drawScene()
    {
        // Update states
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        CameraUtils::updateViewMatrix(mCamera);
       
        drawBezierSurface();

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    void BezierSurfaceTesselationApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
    {
        if ((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.15f * static_cast<float>(x - mLastMousePos.x));
            const float dy = DirectX::XMConvertToRadians(0.15f * static_cast<float>(y - mLastMousePos.y));

            CameraUtils::pitch(dy, mCamera);
            CameraUtils::rotateY(dx, mCamera);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void BezierSurfaceTesselationApp::drawBezierSurface()
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);

        // Set input layout, primitive topology and rasterizer state
        mImmediateContext->IASetInputLayout(Managers::ShadersManager::mBezierSurfaceIL);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

        mImmediateContext->RSSetState(mWireframeMode ? Managers::PipelineStatesManager::mWireframeRS : nullptr);


        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mBezierSurfaceBufferInfo->mVertexBuffer;
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mBezierSurfaceVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mBezierSurfacePS;
        ID3D11HullShader* hullShader = Managers::ShadersManager::mBezierSurfaceHS;
        ID3D11DomainShader* domainShader = Managers::ShadersManager::mBezierSurfaceDS;
        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mBezierSurfaceIL;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mBezierSurfaceBufferInfo->mBaseVertexLocation;
        const uint32_t vertexCount = Managers::GeometryBuffersManager::mBezierSurfaceBufferInfo->mVertexCount;

        mBezierSurfacePSPerFrameBuffer.mData.mDirectionalLight = mDirectionalLight;
        mBezierSurfacePSPerFrameBuffer.mData.mEyePositionW = mCamera.mPosition;
        mBezierSurfacePSPerFrameBuffer.mData.mMaterial = mSandMaterial;

        // Set shaders
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);
        mImmediateContext->HSSetShader(hullShader, nullptr, 0);
        mImmediateContext->DSSetShader(domainShader, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);

        ID3D11Buffer* domainShaderBuffers[] = { mBezierSurfaceDSPerFrameBuffer.mBuffer };
        mImmediateContext->DSSetConstantBuffers(0, 1, domainShaderBuffers);

        ID3D11Buffer* hullShaderBuffers[] = { mBezierSurfaceHSPerFrameBuffer.mBuffer };
        mImmediateContext->HSSetConstantBuffers(0, 1, hullShaderBuffers);

        ID3D11Buffer* pixelShaderBuffers[] = { mBezierSurfacePSPerFrameBuffer.mBuffer };
        mImmediateContext->PSSetConstantBuffers(0, 1, pixelShaderBuffers);

        // Update vertex buffer
        uint32_t stride = sizeof(Geometry::BezierSurfaceVertex);
        uint32_t offset = 0;
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer for land
        //
        DirectX::XMFLOAT3 rotation(0.0, mRotationAmmount, 0.0f); 
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mBezierSurfaceWorld) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation));
        DirectX::XMStoreFloat4x4(&mBezierSurfaceDSPerFrameBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mBezierSurfaceDSPerFrameBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));


        // Update in game time
        mBezierSurfaceDSPerFrameBuffer.mData.mInGameTime = TimerUtils::inGameTime(mTimer);

        // Update tesselation factor
        mBezierSurfaceHSPerFrameBuffer.mData.mTesselationFactor = mTesselationFactor;

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mSandTexTransform);
        DirectX::XMStoreFloat4x4(&mBezierSurfaceDSPerFrameBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        ConstantBufferUtils::applyChanges(*mImmediateContext, mBezierSurfaceDSPerFrameBuffer);
        
        mImmediateContext->PSSetShaderResources(0, 1, &Managers::ResourcesManager::mSandSRV);

        ConstantBufferUtils::applyChanges(*mImmediateContext, mBezierSurfaceHSPerFrameBuffer);
        ConstantBufferUtils::applyChanges(*mImmediateContext, mBezierSurfaceDSPerFrameBuffer);
        ConstantBufferUtils::applyChanges(*mImmediateContext, mBezierSurfacePSPerFrameBuffer);
        mImmediateContext->Draw(vertexCount, baseVertexLocation);
    }
}