#include "InstancingApp.h"
 
#include <DirectXColors.h>
#include <DirectXMath.h>

#include <DxErrorChecker.h>
#include <GeometryGenerator.h>
#include <MathHelper.h>

namespace Framework
{
    void InstancingApp::updateScene(const float dt)
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
        
        updateInstancedBuffer();
    }

    void InstancingApp::drawScene()
    {
        // Update states
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        CameraUtils::updateViewMatrix(mCamera);

        setShapesGeneralSettings();
       
        drawFloor();
        drawCylinder();

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    void InstancingApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
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

    void InstancingApp::setShapesGeneralSettings()
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
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);
    }

    void InstancingApp::updateInstancedBuffer()
    {
        ID3D11Buffer* instancedVertexBuffer = Managers::GeometryBuffersManager::mInstancedBufferInfo->mVertexBuffer;

        D3D11_MAPPED_SUBRESOURCE mappedData; 
        mImmediateContext->Map(instancedVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

        Managers::GeometryBuffersManager::InstancedData* dataView = reinterpret_cast<Managers::GeometryBuffersManager::InstancedData*>(mappedData.pData);

        for(uint32_t i = 0; i < Managers::GeometryBuffersManager::mInstancedBufferInfo->mVertexCount; ++i)
        {
            DirectX::XMFLOAT3 rotation(0.0, mRotationAmmount, 0.0f); 
            DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mCylinderWorld[i].mWorld) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation));
            DirectX::XMFLOAT4X4 newWorld;
            DirectX::XMStoreFloat4x4(&newWorld, world);
        
            dataView[i].mWorld = newWorld;
        }        

        mImmediateContext->Unmap(instancedVertexBuffer, 0);
    }

    void InstancingApp::drawCylinder()
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

        UINT stride[2] = {sizeof(VertexData), sizeof(Managers::GeometryBuffersManager::InstancedData)};
        UINT offset[2] = {0, 0};
        ID3D11Buffer* vertexBuffers[2] = {shapesVertexBuffer, instancedVertexBuffer};
        mImmediateContext->IASetVertexBuffers(0, 2, vertexBuffers, stride, offset);
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        //
        // Update constant buffers
        //
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mViewProjection, DirectX::XMMatrixTranspose(viewProjection));
        
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mCommonTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11Buffer* vertexShaderBuffers = &mShapesVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);

        //
        // Set shader resources
        //
        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mCylinderDiffuseMapSRV, Managers::ResourcesManager::mCylinderNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        //
        // Draw
        //
        const uint32_t indexCountPerInstance = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexCount;
        const uint32_t instanceCount = Managers::GeometryBuffersManager::mInstancedBufferInfo->mVertexCount;
        const uint32_t shapesBaseVertexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mBaseVertexLocation;
        const uint32_t shapesStartIndexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mStartIndexLocation;
        const uint32_t startInstanceLocation = Managers::GeometryBuffersManager::mInstancedBufferInfo->mBaseVertexLocation;
        mImmediateContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, shapesStartIndexLocation, shapesBaseVertexLocation, 0);       
    }

    void InstancingApp::drawFloor()
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

        uint32_t stride = sizeof(VertexData);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update constant buffers
        //
        DirectX::XMFLOAT3 rotation(0.0, mRotationAmmount, 0.0f); 
        DirectX::XMMATRIX world = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation)) * DirectX::XMLoadFloat4x4(&mFloorWorld);
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mCommonTexTransform);
        DirectX::XMStoreFloat4x4(&mFloorVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mFloorVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11Buffer* vertexShaderBuffers = &mFloorVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);

        //
        // Set shader resources
        //
        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mFloorDiffuseMapSRV, Managers::ResourcesManager::mFloorNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        //
        // Draw
        //
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mStartIndexLocation;
        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
  }    
}