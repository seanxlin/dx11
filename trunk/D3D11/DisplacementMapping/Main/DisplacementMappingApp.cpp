#include "DisplacementMappingApp.h"
 
#include <DirectXColors.h>
#include <DirectXMath.h>

#include <DxErrorChecker.h>
#include <GeometryGenerator.h>
#include <MathHelper.h>

namespace
{
    const float gMinTessellationFactor = 0.001f;
    const float gMaxTessellationFactor = 5.0f;
    const float gTessellationOffset = 0.0025f;
}

namespace Framework
{
    void DisplacementMappingApp::updateScene(const float dt)
    {
        //
        // Control the camera.
        //
        if (GetAsyncKeyState('W') & 0x8000)
            mCamera.walk(50.0f * dt);

        if (GetAsyncKeyState('S') & 0x8000)
            mCamera.walk(-50.0f * dt);

        if (GetAsyncKeyState('A') & 0x8000)
            mCamera.strafe(-50.0f * dt);

        if (GetAsyncKeyState('D') & 0x8000)
            mCamera.strafe(50.0f * dt);

        if (GetAsyncKeyState('G') & 0x8000)
            mTesselationFactor = (gMaxTessellationFactor < mTesselationFactor + gTessellationOffset) ? gMaxTessellationFactor : mTesselationFactor + gTessellationOffset;

        if (GetAsyncKeyState('H') & 0x8000) 
            mTesselationFactor = (mTesselationFactor - gTessellationOffset < gMinTessellationFactor) ? gMinTessellationFactor : mTesselationFactor - gTessellationOffset;

        /*if (GetAsyncKeyState('F') & 0x8000) 
            mTesselationFactor = 8.0f;

        if (GetAsyncKeyState('G') & 0x8000) 
            mTesselationFactor = 16.0f;

        if (GetAsyncKeyState('H') & 0x8000) 
            mTesselationFactor = 32.0f;

        if (GetAsyncKeyState('J') & 0x8000) 
            mTesselationFactor = 64.0f;*/

        if (GetAsyncKeyState('T') & 0x8000) 
            mWireframeMode = true;

        if (GetAsyncKeyState('Y') & 0x8000) 
            mWireframeMode = false;

        mRotationAmmount += 0.25f * dt;
    }

    void DisplacementMappingApp::drawScene()
    {
        // Update states
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        mCamera.updateViewMatrix();

        setShapesGeneralSettings();
       
        //drawFloor();w
        drawCylinder();
        //drawSphere();
        //drawBox();

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DebugUtils::DxErrorChecker(result);
    }

    void DisplacementMappingApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
    {
        if ((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.15f * static_cast<float>(x - mLastMousePos.x));
            const float dy = DirectX::XMConvertToRadians(0.15f * static_cast<float>(y - mLastMousePos.y));

            mCamera.pitch(dy);
            mCamera.rotateY(dx);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void DisplacementMappingApp::setShapesGeneralSettings()
    {
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mShapesVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mShapesPS;
        ID3D11HullShader* hullShader = Managers::ShadersManager::mShapesHS;
        ID3D11DomainShader* domainShader = Managers::ShadersManager::mShapesDS;
        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mShapesIL;

        // Set input layout, primitive topology and rasterizer state
        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        //mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mImmediateContext->RSSetState(mWireframeMode ? Managers::PipelineStatesManager::mWireframeRS : nullptr);

        //
        // Update constant buffers
        //
        mShapesHSPerFrameBuffer.mData.mTessellationFactor = mTesselationFactor;
        mShapesHSPerFrameBuffer.applyChanges(mImmediateContext);

        const DirectX::XMMATRIX viewProjection = mCamera.viewProjection();
        DirectX::XMStoreFloat4x4(&mShapesDSPerFrameBuffer.mData.mViewProjection, DirectX::XMMatrixTranspose(viewProjection));
        mShapesDSPerFrameBuffer.mData.mEyePositionW = mCamera.position();
        mShapesDSPerFrameBuffer.applyChanges(mImmediateContext);

        memcpy(&mShapesPSPerFrameBuffer.mData.mDirectionalLight, &mDirectionalLight, sizeof(mDirectionalLight));
        mShapesPSPerFrameBuffer.mData.mEyePositionW = mCamera.position();
        mShapesPSPerFrameBuffer.applyChanges(mImmediateContext);

        mShapesPSPerObjectBuffer.mData.mMaterial = mShapesMaterial;
        mShapesPSPerObjectBuffer.applyChanges(mImmediateContext);

        //
        // Set constant buffers
        //
        ID3D11Buffer* vertexShaderBuffers = mShapesVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);

        ID3D11Buffer* hullShaderBuffer = mShapesHSPerFrameBuffer.buffer();
        mImmediateContext->HSSetConstantBuffers(0, 1, &hullShaderBuffer);

        ID3D11Buffer* domainShaderBuffer = mShapesDSPerFrameBuffer.buffer();
        mImmediateContext->DSSetConstantBuffers(0, 1, &domainShaderBuffer);

        ID3D11Buffer* pixelShaderBuffers[] = { mShapesPSPerFrameBuffer.buffer(), mShapesPSPerObjectBuffer.buffer() };
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        //
        // Set shaders
        //
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);        
        mImmediateContext->HSSetShader(hullShader, nullptr, 0);
        mImmediateContext->DSSetShader(domainShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);
        
        //
        // Set sampler states
        //
        mImmediateContext->DSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);
    }

    void DisplacementMappingApp::drawBox()
    {
        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mBoxBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mBoxBufferInfo->mIndexBuffer;        
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mBoxBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mBoxBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mBoxBufferInfo->mIndexCount;        

        // Update index buffer
        uint32_t stride = sizeof(Geometry::GeometryGenerator::Vertex);
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
        
        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mShapesTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(mImmediateContext);

        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mBoxDiffuseMapSRV, Managers::ResourcesManager::mBoxNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        ID3D11ShaderResourceView* domainShaderResources = Managers::ResourcesManager::mBoxNormalMapSRV;
        mImmediateContext->DSSetShaderResources(0, 1, &domainShaderResources);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void DisplacementMappingApp::drawSphere()
    {
        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mSphereBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mSphereBufferInfo->mIndexBuffer;        
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mSphereBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mSphereBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mSphereBufferInfo->mIndexCount;    

        // Update index buffer
        uint32_t stride = sizeof(Geometry::GeometryGenerator::Vertex);
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
        
        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mShapesTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(mImmediateContext);

        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mSpheresDiffuseMapSRV, Managers::ResourcesManager::mSpheresNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        ID3D11ShaderResourceView* domainShaderResources = Managers::ResourcesManager::mSpheresNormalMapSRV;
        mImmediateContext->DSSetShaderResources(0, 1, &domainShaderResources);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void DisplacementMappingApp::drawCylinder()
    {
        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mCylinderBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexBuffer;        
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mCylinderBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mCylinderBufferInfo->mIndexCount; 

        //
        // Set vertex and index buffers
        //
        uint32_t stride = sizeof(Geometry::GeometryGenerator::Vertex);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer
        //
        DirectX::XMFLOAT3 rotation(0.0, mRotationAmmount, 0.0f); 
        DirectX::XMMATRIX world = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation)) * DirectX::XMLoadFloat4x4(&mCylinderWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));
        
        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mShapesTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(mImmediateContext);

        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mCylinderDiffuseMapSRV, Managers::ResourcesManager::mCylinderNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        ID3D11ShaderResourceView* domainShaderResources = Managers::ResourcesManager::mCylinderNormalMapSRV;
        mImmediateContext->DSSetShaderResources(0, 1, &domainShaderResources);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void DisplacementMappingApp::drawFloor()
    {
        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexBuffer;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexCount;

        // Update index buffer
        uint32_t stride = sizeof(Geometry::GeometryGenerator::Vertex);
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
        
        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mShapesTexTransform);
        DirectX::XMStoreFloat4x4(&mShapesVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mShapesVSPerObjectBuffer.applyChanges(mImmediateContext);
        
        ID3D11ShaderResourceView* pixelShaderResources[] = { Managers::ResourcesManager::mFloorDiffuseMapSRV, Managers::ResourcesManager::mFloorNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        ID3D11ShaderResourceView* domainShaderResources = Managers::ResourcesManager::mFloorNormalMapSRV;
        mImmediateContext->DSSetShaderResources(0, 1, &domainShaderResources);

        // Set pixel shader per object buffer
        mShapesPSPerObjectBuffer.mData.mMaterial = mFloorMaterial;
        mShapesPSPerObjectBuffer.applyChanges(mImmediateContext);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }    
}