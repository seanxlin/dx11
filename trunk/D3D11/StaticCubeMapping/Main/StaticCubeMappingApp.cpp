#include "StaticCubeMappingApp.h"
 
#include <DirectXColors.h>
#include <DirectXMath.h>

#include "HLSL/Vertex.h"

#include <DxErrorChecker.h>
#include <MathHelper.h>

namespace Framework
{
    void StaticCubeMappingApp::updateScene(const float dt)
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

        mRotationAmmount += 0.25f * dt;
    }

    void StaticCubeMappingApp::drawScene()
    {
        // Update states
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        mCamera.updateViewMatrix();
       
        drawLand();
        drawSphere();
        drawSky();

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DebugUtils::DxErrorChecker(result);
    }

    void StaticCubeMappingApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
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

    void StaticCubeMappingApp::drawSphere()
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = mCamera.viewProjection();

        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mSphereBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mSphereBufferInfo->mIndexBuffer;
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mSphereVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mSpherePS;
        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mSphereIL;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mSphereBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mSphereBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mSphereBufferInfo->mIndexCount;

        // Update per frame constant buffers
        mSpherePSPerFrameBuffer.mData.mDirectionalLight = mDirectionalLight;
        mSpherePSPerFrameBuffer.mData.mEyePositionW = mCamera.position();
        mSpherePSPerFrameBuffer.applyChanges(*mImmediateContext);

        // Set pixel shader per object buffer
        mSpherePSPerObjectBuffer.mData.mMaterial = mSphereMaterial;
        mSpherePSPerObjectBuffer.applyChanges(*mImmediateContext);

        // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set shaders
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { &mSpherePSPerFrameBuffer.buffer(), &mSpherePSPerObjectBuffer.buffer() };
        ID3D11Buffer* vertexShaderBuffers = &mSphereVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        // Update index buffer
        uint32_t stride = sizeof(Geometry::CommonVertex);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer for land
        //
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mSphereWorld);

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mSphereVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mSphereVSPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mSphereVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mSandTexTransform);
        DirectX::XMStoreFloat4x4(&mSphereVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mSphereVSPerObjectBuffer.applyChanges(*mImmediateContext);

        ID3D11ShaderResourceView* pixelShaderShaderResources[] = {Managers::ResourcesManager::mSphereDiffuseMapSRV, Managers::ResourcesManager::mSkyCubeMapSRV};
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderShaderResources);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void StaticCubeMappingApp::drawLand()
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = mCamera.viewProjection();

        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mLandBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mLandBufferInfo->mIndexBuffer;
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mLandVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mLandPS;
        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mLandIL;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mLandBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mLandBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mLandBufferInfo->mIndexCount;

        // Update per frame constant buffers for land and billboards
        mLandPerFrameBuffer.mData.mDirectionalLight = mDirectionalLight;
        mLandPerFrameBuffer.mData.mEyePositionW = mCamera.position();
        mLandPerFrameBuffer.applyChanges(*mImmediateContext);

        // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set shaders
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { &mLandPerFrameBuffer.buffer(), &mLandPSPerObjectBuffer.buffer() };
        ID3D11Buffer* vertexShaderBuffers = &mLandVSPerObjectBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderBuffers);
        mImmediateContext->PSSetConstantBuffers(0, 2, pixelShaderBuffers);

        // Update index buffer
        uint32_t stride = sizeof(Geometry::CommonVertex);
        uint32_t offset = 0;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Update vertex buffer
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        //
        // Update per object constant buffer for land
        //
        DirectX::XMFLOAT3 rotation(0.0, mRotationAmmount, 0.0f); 
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mLandWorld) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation));

        // Update world matrix
        DirectX::XMStoreFloat4x4(&mLandVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));

        // Update world * view * projection matrix
        DirectX::XMMATRIX worldViewProjection = world * viewProjection;
        DirectX::XMStoreFloat4x4(&mLandVSPerObjectBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        // Update world inverse transpose matrix
        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mLandVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mSandTexTransform);
        DirectX::XMStoreFloat4x4(&mLandVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        mLandVSPerObjectBuffer.applyChanges(*mImmediateContext);
        
        mImmediateContext->PSSetShaderResources(0, 1, &Managers::ResourcesManager::mSandSRV);

        // Set pixel shader per object buffer
        mLandPSPerObjectBuffer.mData.mMaterial = mSandMaterial;
        mLandPSPerObjectBuffer.applyChanges(*mImmediateContext);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void StaticCubeMappingApp::drawSky()
    {
        // center Sky about eye in world space
        const DirectX::XMFLOAT3 eyePosition = mCamera.position();
        DirectX::XMMATRIX skyTranslation = DirectX::XMMatrixTranslation(eyePosition.x, eyePosition.y, eyePosition.z);

        // Update per frame buffer
        const DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixMultiply(skyTranslation, mCamera.viewProjection());
        DirectX::XMStoreFloat4x4(&mSkyPerFrameBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));
        mSkyPerFrameBuffer.applyChanges(*mImmediateContext);
        ID3D11Buffer* vertexShaderPerFrameBuffer = &mSkyPerFrameBuffer.buffer();
        mImmediateContext->VSSetConstantBuffers(0, 1, &vertexShaderPerFrameBuffer);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);

        // Set resources
        ID3D11ShaderResourceView* cubeMapSRV = Managers::ResourcesManager::mSkyCubeMapSRV;
        mImmediateContext->PSSetShaderResources(0, 1, &cubeMapSRV);

        // Set states
        mImmediateContext->RSSetState(Managers::PipelineStatesManager::mNoCullRS);
        mImmediateContext->OMSetDepthStencilState(Managers::PipelineStatesManager::mLessEqualDSS, 0);

        // Useful info
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mSkyBufferInfo->mVertexBuffer;
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mSkyBufferInfo->mIndexBuffer;
        ID3D11VertexShader* vertexShader = Managers::ShadersManager::mSkyVS;
        ID3D11PixelShader* pixelShader = Managers::ShadersManager::mSkyPS;
        ID3D11InputLayout* inputLayout = Managers::ShadersManager::mSkyIL;
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mSkyBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mSkyBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mSkyBufferInfo->mIndexCount;
        
        // Set shaders
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        uint32_t stride = sizeof(Geometry::SkyVertex);
        uint32_t offset = 0;
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);

        // Set defaults
        mImmediateContext->VSSetShader(nullptr, nullptr, 0);
        mImmediateContext->PSSetShader(nullptr, nullptr, 0);
        mImmediateContext->IASetVertexBuffers(0, 0, nullptr, &stride, &offset);
        mImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
        mImmediateContext->IASetInputLayout(nullptr);
        mImmediateContext->RSSetState(nullptr);
        mImmediateContext->OMSetDepthStencilState(nullptr, 0);
        mImmediateContext->PSSetSamplers(0, 0, nullptr);
    }
}