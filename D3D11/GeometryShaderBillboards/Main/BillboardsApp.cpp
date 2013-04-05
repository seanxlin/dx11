#include "BillboardsApp.h"
 
#include <DirectXColors.h>
#include <DirectXMath.h>

#include "HLSL/Vertex.h"

#include <DxErrorChecker.h>
#include <MathHelper.h>

namespace Framework
{
    void BillboardsApp::updateScene(const float dt)
    {
        //
        // Update eye position and view matrix.
        //

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
        // Animate the lights.
        //

        // The spotlight takes on the camera position and is aimed in the
        // same direction the camera is looking.  In this way, it looks
        // like we are holding a flashlight.
        mSpotLight.mPosition = mEyePositionW;
        DirectX::XMVECTOR substraction = DirectX::XMVectorSubtract(target, pos);
        DirectX::XMStoreFloat3(&mSpotLight.mDirection, DirectX::XMVector3Normalize(substraction));

        //
        // Switch the render mode based in key input.
        //
        if (GetAsyncKeyState('R') & 0x8000)
            mAlphaToCoverageOn = true;

        if (GetAsyncKeyState('T') & 0x8000)
            mAlphaToCoverageOn = false;
    }

    void BillboardsApp::drawScene()
    {
        // Update states
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Compute view * projection matrix
        const DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&mView);
        const DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&mProjection);
        const DirectX::XMMATRIX viewProjection = view * projection;

        // Update per frame constant buffers for land and billboards
        mCommonPerFrameBuffer.mData.mDirectionalLight = mDirectionalLight;
        mCommonPerFrameBuffer.mData.mSpotLight = mSpotLight;
        mCommonPerFrameBuffer.mData.mEyePositionW = mEyePositionW;
        ConstantBufferUtils::applyChanges(*mImmediateContext, mCommonPerFrameBuffer);

        drawBillboards(viewProjection);
        drawLand(viewProjection);

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    void BillboardsApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
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

    void BillboardsApp::drawBillboards(DirectX::CXMMATRIX viewProjection)
    {
         // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(Managers::ShadersManager::mBillboardsIL);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

        // Set shaders
        mImmediateContext->VSSetShader(Managers::ShadersManager::mBillboardsVS, nullptr, 0);
        mImmediateContext->GSSetShader(Managers::ShadersManager::mBillboardsGS, nullptr, 0);
        mImmediateContext->PSSetShader(Managers::ShadersManager::mBillboardsPS, nullptr, 0);
        
        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);
        
        // Set constant buffers
        ID3D11Buffer* billboardsConstantBuffer[] = { mCommonPerFrameBuffer.mBuffer, mBillboardsPerObjectBuffer.mBuffer };
        mImmediateContext->GSSetConstantBuffers(0, 2, billboardsConstantBuffer);
        mImmediateContext->PSSetConstantBuffers(0, 2, billboardsConstantBuffer);

        // Get needed info about geometry buffers.
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mBillboardsBufferInfo->mVertexBuffer;
        uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mBillboardsBufferInfo->mBaseVertexLocation;
        const uint32_t vertexCount = Managers::GeometryBuffersManager::mBillboardsBufferInfo->mVertexCount;
        
        // Update index and vertex buffers
        uint32_t stride = sizeof(Geometry::BillboardVertex);
        uint32_t offset = 0;
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        // Update pero object constant buffer
        DirectX::XMStoreFloat4x4(&mBillboardsPerObjectBuffer.mData.mViewProjection, DirectX::XMMatrixTranspose(viewProjection));
        mBillboardsPerObjectBuffer.mData.mMaterial = mPalmMaterial;
        ConstantBufferUtils::applyChanges(*mImmediateContext, mBillboardsPerObjectBuffer);

        // Set blend state
        float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f}; 
        if(mAlphaToCoverageOn)
            mImmediateContext->OMSetBlendState(Managers::PipelineStatesManager::mAlphaToCoverageBS, blendFactor, 0xffffffff);
        
        // Set shader resources
        mImmediateContext->PSSetShaderResources(0, 1, &Managers::ResourcesManager::mPalmsSRV);

        // Draw
        mImmediateContext->Draw(vertexCount, baseVertexLocation);

        // Set defaults
        mImmediateContext->GSSetShader(nullptr, nullptr, 0);
        mImmediateContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
    }

    void BillboardsApp::drawLand(DirectX::CXMMATRIX viewProjection)
    {
        // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(Managers::ShadersManager::mLandIL);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set shaders
        mImmediateContext->VSSetShader(Managers::ShadersManager::mLandVS, nullptr, 0);
        mImmediateContext->PSSetShader(Managers::ShadersManager::mLandPS, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { mCommonPerFrameBuffer.mBuffer, mLandPerObjectBuffer.mBuffer };
        ID3D11Buffer* vertexShaderBuffers = mLandPerObjectBuffer.mBuffer;
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        // Get needed info about geometry buffers.
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mLandBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mLandBufferInfo->mIndexBuffer;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mLandBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mLandBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mLandBufferInfo->mIndexCount;

        // Update index buffer
        uint32_t stride = sizeof(Geometry::LandVertex);
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
        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mLandPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mSandTexTransform);
        DirectX::XMStoreFloat4x4(&mLandPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));

        // Set land material and textures
        mLandPerObjectBuffer.mData.mMaterial = mSandMaterial;
        mImmediateContext->PSSetShaderResources(0, 1, &Managers::ResourcesManager::mSandSRV);

        // Apply buffer changes and draw.
        ConstantBufferUtils::applyChanges(*mImmediateContext, mLandPerObjectBuffer);
        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }
}