#include "ShadowMappingApp.h"
 
#include <DirectXColors.h>
#include <DirectXMath.h>

#include <DxErrorChecker.h>
#include <GeometryGenerator.h>
#include <MathHelper.h>

namespace Framework
{
    void ShadowMappingApp::updateScene(const float dt)
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

        //
        // Animate the lights (and hence shadows).
        //
        mLightRotationAngle += 0.25f * dt;

        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(mLightRotationAngle);
        for(uint32_t i = 0; i < 3; ++i)
        {
            DirectX::XMVECTOR lightDirection = DirectX::XMLoadFloat3(&mDefaultShadowLightDirection[i]);
            lightDirection =  DirectX::XMVector3TransformNormal(lightDirection, rotationMatrix);
            XMStoreFloat3(&mDirectionalLight[i].mDirection, lightDirection);
        }

        buildShadowTransform();
    }

    void ShadowMappingApp::drawScene()
    {
        mShadowMapper->bindDepthStencilViewAndSetNullRenderTarget(*mImmediateContext);

        drawSceneToShadowMap();

        // Update states
        mImmediateContext->RSSetState(nullptr);

        //
        // Restore the back and depth buffer to the OM stage.
        //
        ID3D11RenderTargetView* renderTargets[] = { mRenderTargetView };
        mImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
        mImmediateContext->RSSetViewports(1, &mScreenViewport);

        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        Utils::CameraUtils::updateViewMatrix(mCamera);

        setShapesGeneralSettings();
       
        drawFloor();
        drawCylinders();

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DebugUtils::DxErrorChecker(result);
    }

    void ShadowMappingApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
    {
        if ((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.15f * static_cast<float>(x - mLastMousePos.x));
            const float dy = DirectX::XMConvertToRadians(0.15f * static_cast<float>(y - mLastMousePos.y));

            Utils::CameraUtils::pitch(dy, mCamera);
            Utils::CameraUtils::rotateY(dx, mCamera);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void ShadowMappingApp::setShapesGeneralSettings()
    {
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        //
        // Set constant buffers
        //
        ID3D11Buffer* pixelShaderBuffers[] = { &mCommonPSPerFrameBuffer.buffer(), &mCommonPSPerObjectBuffer.buffer() };
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        //
        // Update common buffers
        //
        memcpy(&mCommonPSPerFrameBuffer.mData.mDirectionalLight, &mDirectionalLight, sizeof(mDirectionalLight));
        mCommonPSPerFrameBuffer.mData.mEyePositionW = mCamera.mPosition;
        mCommonPSPerFrameBuffer.applyChanges(*mImmediateContext);

        mCommonPSPerObjectBuffer.mData.mMaterial = mShapesMaterial;
        mCommonPSPerObjectBuffer.applyChanges(*mImmediateContext);
        
        //
        // Set sampler states
        //
        ID3D11SamplerState* pixelShaderSamplerStates[] = { Managers::PipelineStatesManager::mAnisotropicSS };
        mImmediateContext->PSSetSamplers(0, 1, pixelShaderSamplerStates);
    }

    void ShadowMappingApp::drawCylinders()
    {
        //
        // Set shaders and input layout
        //
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mShapesVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mShapesPS;
        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mShapesIL;

        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);
                
        //
        // Set geometry buffers
        //
        ID3D11Buffer* shapesVertexBuffer = Managers::GeometryBuffersManager::mCylinderBufferInfo->mVertexBuffer;
        ID3D11Buffer* instancedVertexBuffer = Managers::GeometryBuffersManager::mInstancedBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexBuffer;        
        const uint32_t indexCount = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexCount;        

        UINT stride[2] = {sizeof(Geometry::VertexData), sizeof(Managers::GeometryBuffersManager::InstancedData)};
        UINT offset[2] = {0, 0};
        ID3D11Buffer* vertexBuffers[2] = {shapesVertexBuffer, instancedVertexBuffer};
        mImmediateContext->IASetVertexBuffers(0, 2, vertexBuffers, stride, offset);
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        //
        // Update constant buffers
        //
        const DirectX::XMMATRIX viewProjection = Utils::CameraUtils::computeViewProjectionMatrix(mCamera);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mViewProjection, DirectX::XMMatrixTranspose(viewProjection));
        
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mCommonTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        
        texTransform = DirectX::XMLoadFloat4x4(&mShadowTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mShadowTransform, DirectX::XMMatrixTranspose(texTransform));
        
        mShapesVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11Buffer* vertexShaderBuffers = &mShapesVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);

        //
        // Set shader resources
        //
        ID3D11ShaderResourceView* pixelShaderResources[] = { 
            Managers::ResourcesManager::mCylinderDiffuseMapSRV, 
            Managers::ResourcesManager::mCylinderNormalMapSRV,
            &mShadowMapper->depthMapSRV(),
        };
        mImmediateContext->PSSetShaderResources(0, 3, pixelShaderResources);

        //
        // Draw
        //
        const uint32_t indexCountPerInstance = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexCount;
        const uint32_t instanceCount = Managers::GeometryBuffersManager::mInstancedBufferInfo->mVertexCount;
        const uint32_t shapesBaseVertexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mBaseVertexLocation;
        const uint32_t shapesStartIndexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mStartIndexLocation;
        const uint32_t startInstanceLocation = Managers::GeometryBuffersManager::mInstancedBufferInfo->mBaseVertexLocation;
        mImmediateContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, shapesStartIndexLocation, shapesBaseVertexLocation, startInstanceLocation);       
    }

    void ShadowMappingApp::drawCylindersIntoShadowMap()
    {
        //
        // Compute light's view projection matrix
        //
        DirectX::XMMATRIX lightView = DirectX::XMLoadFloat4x4(&mLightView);
        DirectX::XMMATRIX lightProjection = DirectX::XMLoadFloat4x4(&mLightProjection);
        DirectX::XMMATRIX lightViewProjection = DirectX::XMMatrixMultiply(lightView, lightProjection);

        //
        // Set shaders
        //
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mShadowMapVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mShadowMapPS;

        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        //
        // Set geometry buffers
        //
        ID3D11Buffer* shapesVertexBuffer = Managers::GeometryBuffersManager::mCylinderBufferInfo->mVertexBuffer;
        ID3D11Buffer* instancedVertexBuffer = Managers::GeometryBuffersManager::mInstancedBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexBuffer;        
        const uint32_t indexCount = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexCount;        

        UINT stride[2] = {sizeof(Geometry::VertexData), sizeof(Managers::GeometryBuffersManager::InstancedData)};
        UINT offset[2] = {0, 0};
        ID3D11Buffer* vertexBuffers[2] = {shapesVertexBuffer, instancedVertexBuffer};
        mImmediateContext->IASetVertexBuffers(0, 2, vertexBuffers, stride, offset);
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        //
        // Update constant buffers
        //
        DirectX::XMStoreFloat4x4(&mShadowMapVSPerObjectBuffer.mData.mLightViewProjection, DirectX::XMMatrixTranspose(lightViewProjection));

        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mCommonTexTransform);
        DirectX::XMStoreFloat4x4(&mShadowMapVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));

        mShadowMapVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11Buffer* vertexShaderBuffers = &mShadowMapVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);

        //
        // Set shader resources
        //
        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mCylinderDiffuseMapSRV };
        mImmediateContext->PSSetShaderResources(0, 1, pixelShaderResources);

        //
        // Set sampler states
        //
        ID3D11SamplerState* pixelShaderSamplerStates[] = { Managers::PipelineStatesManager::mAnisotropicSS };
        mImmediateContext->PSSetSamplers(0, 1, pixelShaderSamplerStates);

        //
        // Draw
        //
        const uint32_t indexCountPerInstance = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexCount;
        const uint32_t instanceCount = Managers::GeometryBuffersManager::mInstancedBufferInfo->mVertexCount;
        const uint32_t shapesBaseVertexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mBaseVertexLocation;
        const uint32_t shapesStartIndexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mStartIndexLocation;
        const uint32_t startInstanceLocation = Managers::GeometryBuffersManager::mInstancedBufferInfo->mBaseVertexLocation;
        mImmediateContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, shapesStartIndexLocation, shapesBaseVertexLocation, startInstanceLocation);       
    }

    void ShadowMappingApp::drawFloorIntoShadowMap()
    {
        //
        // Compute light's view projection matrix
        //
        DirectX::XMMATRIX lightView = DirectX::XMLoadFloat4x4(&mLightView);
        DirectX::XMMATRIX lightProjection = DirectX::XMLoadFloat4x4(&mLightProjection);
        DirectX::XMMATRIX lightViewProjection = DirectX::XMMatrixMultiply(lightView, lightProjection);

        //
        // Set shaders
        //
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mFloorShadowMapVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mFloorShadowMapPS;

        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        //
        // Set geometry buffers
        //
        ID3D11Buffer* shapesVertexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexBuffer;        
        const uint32_t indexCount = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexCount;        

        UINT stride[1] = {sizeof(Geometry::VertexData)};
        UINT offset[1] = {0};
        ID3D11Buffer* vertexBuffers[1] = {shapesVertexBuffer};
        mImmediateContext->IASetVertexBuffers(0, 1, vertexBuffers, stride, offset);
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        //
        // Update constant buffers
        //
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mFloorWorld);
        DirectX::XMStoreFloat4x4(&mFloorShadowMapVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        DirectX::XMStoreFloat4x4(&mFloorShadowMapVSPerObjectBuffer.mData.mLightViewProjection, DirectX::XMMatrixTranspose(lightViewProjection));

        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mCommonTexTransform);
        DirectX::XMStoreFloat4x4(&mFloorShadowMapVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));

        mFloorShadowMapVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11Buffer* vertexShaderBuffers = &mFloorShadowMapVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);

        //
        // Set shader resources
        //
        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mFloorDiffuseMapSRV };
        mImmediateContext->PSSetShaderResources(0, 1, pixelShaderResources);

        //
        // Set sampler states
        //
        ID3D11SamplerState* pixelShaderSamplerStates[] = { Managers::PipelineStatesManager::mAnisotropicSS };
        mImmediateContext->PSSetSamplers(0, 1, pixelShaderSamplerStates);

        //
        // Draw
        //
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mStartIndexLocation;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mBaseVertexLocation;
                
        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);       
    }

    void ShadowMappingApp::drawFloor()
    {
        //
        // Set shaders and input layout
        //
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mFloorVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mFloorPS;
        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mFloorIL;

        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        //
        // Set geometry buffers
        //
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexBuffer;        
        const uint32_t indexCount = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexCount;        

        uint32_t stride = sizeof(Geometry::VertexData);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update constant buffers
        //
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mFloorWorld);
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        const DirectX::XMMATRIX viewProjection = Utils::CameraUtils::computeViewProjectionMatrix(mCamera);
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mCommonTexTransform);
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));

        texTransform = DirectX::XMLoadFloat4x4(&mShadowTransform);
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mShadowTransform, DirectX::XMMatrixTranspose(texTransform));

        mFloorVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11Buffer* vertexShaderBuffers = &mFloorVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);

        //
        // Set shader resources
        //
        ID3D11ShaderResourceView* pixelShaderResources[] = { 
            Managers::ResourcesManager::mFloorDiffuseMapSRV, 
            Managers::ResourcesManager::mFloorNormalMapSRV,
            &mShadowMapper->depthMapSRV(),
        };
        mImmediateContext->PSSetShaderResources(0, 3, pixelShaderResources);

        //
        // Draw
        //
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mStartIndexLocation;
        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    } 

    void ShadowMappingApp::drawSceneToShadowMap()
    {
        // Set rasterizer state
        mImmediateContext->RSSetState(Managers::PipelineStatesManager::mDepthRS);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mShadowMapIL;
        mImmediateContext->IASetInputLayout(inputLayout);
        drawCylindersIntoShadowMap();

        inputLayout = Managers::ShadersManager::mFloorShadowMapIL;
        mImmediateContext->IASetInputLayout(inputLayout);
        drawFloorIntoShadowMap();
    }

    void ShadowMappingApp::buildShadowTransform()
    {
        // Only the first "main" light casts a shadow.
        DirectX::XMVECTOR lightDirection = DirectX::XMLoadFloat3(&mDirectionalLight[0].mDirection);

        const float scalar = -2.0f * mSceneBounds.mRadius;
        DirectX::XMFLOAT3 scalars(scalar, scalar, scalar);
        DirectX::XMVECTOR lightPosition = DirectX::XMVectorMultiply(lightDirection, DirectX::XMLoadFloat3(&scalars));
        DirectX::XMVECTOR targetPosition = DirectX::XMLoadFloat3(&mSceneBounds.mCenter);
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(lightPosition, targetPosition, up);

        // Transform bounding sphere to light space.
        DirectX::XMFLOAT3 sphereCenterLS;
        DirectX::XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPosition, view));

        // Ortho frustum in light space encloses scene.
        float leftPlane = sphereCenterLS.x - mSceneBounds.mRadius;
        float bottomPlane = sphereCenterLS.y - mSceneBounds.mRadius;
        float nearPlane = sphereCenterLS.z - mSceneBounds.mRadius;
        float rightPlane = sphereCenterLS.x + mSceneBounds.mRadius;
        float topPlane = sphereCenterLS.y + mSceneBounds.mRadius;
        float farPlane = sphereCenterLS.z + mSceneBounds.mRadius;
        DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);

        // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
        DirectX::XMMATRIX transformMatrix(
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, -0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f);

        DirectX::XMMATRIX shadowTransform = view * projection * transformMatrix;

        DirectX::XMStoreFloat4x4(&mLightView, view);
        DirectX::XMStoreFloat4x4(&mLightProjection, projection);
        DirectX::XMStoreFloat4x4(&mShadowTransform, shadowTransform);
    }
}