#include "NormalMappingApp.h"
 
#include <DirectXColors.h>
#include <DirectXMath.h>

#include <DxErrorChecker.h>
#include <GeometryGenerator.h>
#include <MathHelper.h>

namespace Framework
{
    void NormalMappingApp::updateScene(const float dt)
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

        mRotationAmmount += 0.25f * dt;
    }

    void NormalMappingApp::drawScene()
    {
        // Update states
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        CameraUtils::updateViewMatrix(mCamera);

        setShapesGeneralSettings();
       
        drawFloor();
        drawCylinder();
        drawSphere();
        drawBox();

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    void NormalMappingApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
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

    void NormalMappingApp::setShapesGeneralSettings()
    {
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mShapesVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mShapesPS;
        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mShapesIL;

        // Update per frame constant buffers
        memcpy(&mShapesPSPerFrameBuffer.mData.mDirectionalLight, &mDirectionalLight, sizeof(mDirectionalLight));
        mShapesPSPerFrameBuffer.mData.mEyePositionW = mCamera.mPosition;
        mShapesPSPerFrameBuffer.applyChanges(*mImmediateContext);

        // Set pixel shader per object buffer
        mShapesPSPerObjectBuffer.mData.mMaterial = mShapesMaterial;
        mShapesPSPerObjectBuffer.applyChanges(*mImmediateContext);

        // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set shaders
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);
    }

    void NormalMappingApp::drawBox()
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);

        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mBoxBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mBoxBufferInfo->mIndexBuffer;        
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mBoxBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mBoxBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mBoxBufferInfo->mIndexCount;        

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { &mShapesPSPerFrameBuffer.buffer(), &mShapesPSPerObjectBuffer.buffer() };
        ID3D11Buffer* vertexShaderBuffers = &mShapesVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        // Update index buffer
        uint32_t stride = sizeof(Geometry::VertexData);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer for land
        //
        DirectX::XMFLOAT3 rotation(0.0, mRotationAmmount, 0.0f); 
        DirectX::XMMATRIX world = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation)) * DirectX::XMLoadFloat4x4(&mBoxWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mShapesTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mBoxDiffuseMapSRV, Managers::ResourcesManager::mBoxNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void NormalMappingApp::drawSphere()
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);

        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mSphereBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mSphereBufferInfo->mIndexBuffer;        
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mSphereBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mSphereBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mSphereBufferInfo->mIndexCount;        

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { &mShapesPSPerFrameBuffer.buffer(), &mShapesPSPerObjectBuffer.buffer() };
        ID3D11Buffer* vertexShaderBuffers = &mShapesVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        // Update index buffer
        uint32_t stride = sizeof(Geometry::VertexData);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer for land
        //
        DirectX::XMFLOAT3 rotation(0.0, mRotationAmmount, 0.0f); 
        DirectX::XMMATRIX world = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation)) * DirectX::XMLoadFloat4x4(&mSphereWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mShapesTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mSpheresDiffuseMapSRV, Managers::ResourcesManager::mSpheresNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void NormalMappingApp::drawCylinder()
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);

        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mCylinderBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexBuffer;        
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexCount;        

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { &mShapesPSPerFrameBuffer.buffer(), &mShapesPSPerObjectBuffer.buffer() };
        ID3D11Buffer* vertexShaderBuffers = &mShapesVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        // Update index buffer
        uint32_t stride = sizeof(Geometry::VertexData);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer for land
        //
        DirectX::XMFLOAT3 rotation(0.0, mRotationAmmount, 0.0f); 
        DirectX::XMMATRIX world = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation)) * DirectX::XMLoadFloat4x4(&mCylinderWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mShapesTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mCylinderDiffuseMapSRV, Managers::ResourcesManager::mCylinderNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void NormalMappingApp::drawFloor()
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);

        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexBuffer;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexCount;

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { &mShapesPSPerFrameBuffer.buffer(), &mShapesPSPerObjectBuffer.buffer() };
        ID3D11Buffer* vertexShaderBuffers = &mShapesVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        // Update index buffer
        uint32_t stride = sizeof(Geometry::VertexData);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer
        //
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mFloorWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mShapesTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(*mImmediateContext);
        
        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mFloorDiffuseMapSRV, Managers::ResourcesManager::mFloorNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        // Set pixel shader per object buffer
        mShapesPSPerObjectBuffer.mData.mMaterial = mFloorMaterial;
        mShapesPSPerObjectBuffer.applyChanges(*mImmediateContext);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }    
}