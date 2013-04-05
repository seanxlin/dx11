#include "DynamicCubeMappingApp.h"
 
#include <DirectXColors.h>
#include <DirectXMath.h>

#include "HLSL/Vertex.h"

#include <DxErrorChecker.h>
#include <MathHelper.h>

namespace Framework
{
    void DynamicCubeMappingApp::updateScene(const float dt)
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

    void DynamicCubeMappingApp::drawScene()
    {
        ID3D11RenderTargetView* renderTargets[1];

        // Generate the cube map.
        mImmediateContext->RSSetViewports(1, &mCubeMapViewport);
        ID3D11DepthStencilView* depthStencilView = Managers::ResourcesManager::mDynamicCubeMapDSV;
        for(size_t i = 0; i < 6; ++i)
        {
            ID3D11RenderTargetView* currentRTV = Managers::ResourcesManager::mDynamicCubeMapRTV[i];

            // Clear cube map face and depth buffer.
            mImmediateContext->ClearRenderTargetView(currentRTV, reinterpret_cast<const float*>(&DirectX::Colors::Black));
            mImmediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

            // Bind cube map face as render target.
            renderTargets[0] = currentRTV;
            mImmediateContext->OMSetRenderTargets(1, renderTargets, depthStencilView);

            // Draw the scene with the exception of the center sphere to this cube map face.
            drawScene(mCubeMapCamera[i], false);
        }

        // Restore old viewport and render targets.
        mImmediateContext->RSSetViewports(1, &mScreenViewport);
        renderTargets[0] = mRenderTargetView;
        mImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

        // Have hardware generate lower mipmap levels of cube map.
        ID3D11ShaderResourceView* dynamicCubeMapSRV = Managers::ResourcesManager::mDynamicCubeMapSRV;
        mImmediateContext->GenerateMips(dynamicCubeMapSRV);

        // Now draw the scene as normal, but with the center sphere.
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

        drawScene(mCamera, true);

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    void DynamicCubeMappingApp::drawScene(Camera& camera, const bool isSphereDrawable)
    {
        CameraUtils::updateViewMatrix(mCamera);

        drawLand(camera);

        if (isSphereDrawable)
        {
            drawSphere(camera);
        }

        drawSky(camera);
    }

    void DynamicCubeMappingApp::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
    {
        if ((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.35f * static_cast<float>(x - mLastMousePos.x));
            const float dy = DirectX::XMConvertToRadians(0.35f * static_cast<float>(y - mLastMousePos.y));

            CameraUtils::pitch(dy, mCamera);
            CameraUtils::rotateY(dx, mCamera);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    void DynamicCubeMappingApp::drawSphere(Camera& camera)
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(camera);

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
        mSpherePSPerFrameBuffer.mData.mEyePositionW = camera.mPosition;
        ConstantBufferUtils::applyChanges(*mImmediateContext, mSpherePSPerFrameBuffer);

        // Set pixel shader per object buffer
        mSpherePSPerObjectBuffer.mData.mMaterial = mSphereMaterial;
        ConstantBufferUtils::applyChanges(*mImmediateContext, mSpherePSPerObjectBuffer);

        // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set shaders
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { mSpherePSPerFrameBuffer.mBuffer, mSpherePSPerObjectBuffer.mBuffer };
        ID3D11Buffer* vertexShaderBuffers = mSphereVSPerObjectBuffer.mBuffer;
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
        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mSphereVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mSandTexTransform);
        DirectX::XMStoreFloat4x4(&mSphereVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        ConstantBufferUtils::applyChanges(*mImmediateContext, mSphereVSPerObjectBuffer);

        ID3D11ShaderResourceView* pixelShaderShaderResources[] = {Managers::ResourcesManager::mSphereDiffuseMapSRV, Managers::ResourcesManager::mDynamicCubeMapSRV};
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderShaderResources);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void DynamicCubeMappingApp::buildCubeFaceCamera(const float x, const float y, const float z)
    {
        // Generate the cube map about the given position.
        const DirectX::XMFLOAT3 center(x, y, z);
        const DirectX::XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

        // Look along each coordinate axis.
        DirectX::XMFLOAT3 targets[6] = 
        {
            DirectX::XMFLOAT3(x + 1.0f, y, z), // +X
            DirectX::XMFLOAT3(x - 1.0f, y, z), // -X
            DirectX::XMFLOAT3(x, y + 1.0f, z), // +Y
            DirectX::XMFLOAT3(x, y - 1.0f, z), // -Y
            DirectX::XMFLOAT3(x, y, z + 1.0f), // +Z
            DirectX::XMFLOAT3(x, y, z - 1.0f)  // -Z
        };

        // Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
        // are looking down +Y or -Y, so we need a different "up" vector.
        DirectX::XMFLOAT3 ups[6] = 
        {
            DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
            DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
            DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
            DirectX::XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
            DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
            DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)	 // -Z
        };

        for(size_t i = 0; i < 6; ++i)
        {
            CameraUtils::lookAt(center, targets[i], ups[i], mCubeMapCamera[i]);
            CameraUtils::setFrustrum(0.25f * DirectX::XM_PI, 1.0f, 0.1f, 1000.0f, mCubeMapCamera[i]);
            CameraUtils::updateViewMatrix(mCubeMapCamera[i]);
        }
    }

    void DynamicCubeMappingApp::drawLand(Camera& camera)
    {
        // Compute view * projection matrix
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(camera);

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
        mLandPerFrameBuffer.mData.mEyePositionW = camera.mPosition;
        ConstantBufferUtils::applyChanges(*mImmediateContext, mLandPerFrameBuffer);

        // Set input layout and primitive topology.
        mImmediateContext->IASetInputLayout(inputLayout);
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set shaders
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        // Set sampler states
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mAnisotropicSS);

        // Set constant buffers
        ID3D11Buffer* pixelShaderBuffers[] = { mLandPerFrameBuffer.mBuffer, mLandPSPerObjectBuffer.mBuffer };
        ID3D11Buffer* vertexShaderBuffers = mLandVSPerObjectBuffer.mBuffer;
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
        DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&mLandVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));

        // Update texture transform matrix.
        DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&mSandTexTransform);
        DirectX::XMStoreFloat4x4(&mLandVSPerObjectBuffer.mData.mTexTransform, DirectX::XMMatrixTranspose(texTransform));
        ConstantBufferUtils::applyChanges(*mImmediateContext, mLandVSPerObjectBuffer);
        
        mImmediateContext->PSSetShaderResources(0, 1, &Managers::ResourcesManager::mSandSRV);

        // Set pixel shader per object buffer
        mLandPSPerObjectBuffer.mData.mMaterial = mSandMaterial;
        ConstantBufferUtils::applyChanges(*mImmediateContext, mLandPSPerObjectBuffer);

        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void DynamicCubeMappingApp::drawSky(Camera& camera)
    {
        // center Sky about eye in world space
        const DirectX::XMFLOAT3 eyePosition = camera.mPosition;
        DirectX::XMMATRIX skyTranslation = DirectX::XMMatrixTranslation(eyePosition.x, eyePosition.y, eyePosition.z);

        // Update per frame buffer
        const DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixMultiply(skyTranslation, CameraUtils::computeViewProjectionMatrix(camera));
        DirectX::XMStoreFloat4x4(&mSkyPerFrameBuffer.mData.mWorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));
        ConstantBufferUtils::applyChanges(*mImmediateContext, mSkyPerFrameBuffer);
        ID3D11Buffer* vertexShaderPerFrameBuffer = mSkyPerFrameBuffer.mBuffer;
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