#include "GPUAcceleratedInterlockingTilesAlgorithmNaive.h"

#include "Managers/GeometryBuffersManager.h"
#include "Managers/PipelineStatesManager.h"
#include "Managers/ResourcesManager.h"
#include "Managers/ShadersManager.h"

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <DxErrorChecker.h>
#include <GeometryGenerator.h>
#include <MathHelper.h>

GPUAcceleratedInterlockingTilesAlgorithmNaive::GPUAcceleratedInterlockingTilesAlgorithmNaive()
    : D3DApplication()
    , mWireframeMode(false)
{
    mMainWindowCaption = L"GPU Accelerated Interlocking Tiles Algorithm Naive Demo";

    mCamera.mPosition = DirectX::XMFLOAT3(0.0f, 2.0f, -15.0f);

    mLastMousePosition.x = 0;
    mLastMousePosition.y = 0;

    // Terrain world matrix
    const DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
    DirectX::XMStoreFloat4x4(&mWorldMatrix, translation);

    // Texture scale matrix
    const DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 1.0f, 0.0f);
    DirectX::XMStoreFloat4x4(&mTextureScaleMatrix, scale);

    // Directional lights.
    mDirectionalLight[0].mAmbient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    mDirectionalLight[0].mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mDirectionalLight[0].mSpecular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
    mDirectionalLight[0].mDirection = DirectX::XMFLOAT3(0.707f, -0.707f, 0.0f);

    mDirectionalLight[1].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirectionalLight[1].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirectionalLight[1].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirectionalLight[1].mDirection = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    mDirectionalLight[2].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirectionalLight[2].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirectionalLight[2].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirectionalLight[2].mDirection = DirectX::XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);

    // Initialize material
    mTerrainMaterial.mAmbient  = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mTerrainMaterial.mDiffuse  = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mTerrainMaterial.mSpecular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
    mTerrainMaterial.mReflect  = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

GPUAcceleratedInterlockingTilesAlgorithmNaive::~GPUAcceleratedInterlockingTilesAlgorithmNaive()
{
    ShaderResourcesUtils::destroyAll(gShaderResources);
    ShadersUtils::destroyAll(gShaders);
    PipelineStatesUtils::destroyAll(gPipelineStates);
    GeometryBuffersUtils::destroyAll(gGeometryBuffers);
}

bool GPUAcceleratedInterlockingTilesAlgorithmNaive::init(Direct3DData& direct3DData, WindowData& windowData)
{
    if(!initMainWindow(windowData) || !initDirect3D(direct3DData, windowData))
        return false;

    assert(direct3DData.mDevice);
    ConstantBufferUtils::initialize(*direct3DData.mDevice, mGridPSPerFrameBuffer);
    ConstantBufferUtils::initialize(*direct3DData.mDevice, mGridHSPerFrameBuffer);
    ConstantBufferUtils::initialize(*direct3DData.mDevice, mGridDSPerFrameBuffer);
    ConstantBufferUtils::initialize(*direct3DData.mDevice, mGridVSPerObjectBuffer);
        
    assert(direct3DData.mImmediateContext);
    ShadersUtils::initAll(*direct3DData.mDevice, gShaders);   
    ShaderResourcesUtils::initAll(
        *direct3DData.mDevice, 
        *direct3DData.mImmediateContext, 
        gShaderResources);
    PipelineStatesUtils::initAll(*direct3DData.mDevice, gPipelineStates);
    GeometryBuffersUtils::initAll(*direct3DData.mDevice, gGeometryBuffers);

    return true;
}

void GPUAcceleratedInterlockingTilesAlgorithmNaive::updateScene(const float dt)
{
    //
    // Control the camera.
    //
    const float offset = 50.0f;
    if (GetAsyncKeyState('W') & 0x8000)
    {
        CameraUtils::walk(offset * dt, mCamera);
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        CameraUtils::walk(-offset * dt, mCamera);
    }

    if (GetAsyncKeyState('A') & 0x8000)
    {
        CameraUtils::strafe(-offset * dt, mCamera);
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        CameraUtils::strafe(offset * dt, mCamera);
    }

    if (GetAsyncKeyState('T') & 0x8000) 
    {
        mWireframeMode = true;
    }

    if (GetAsyncKeyState('Y') & 0x8000) 
    {
        mWireframeMode = false;
    }
}

void GPUAcceleratedInterlockingTilesAlgorithmNaive::drawScene(Direct3DData& direct3DData)
{
    direct3DData.mImmediateContext->ClearRenderTargetView(direct3DData.mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Silver));
    direct3DData.mImmediateContext->ClearDepthStencilView(direct3DData.mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    direct3DData.mImmediateContext->RSSetState(mWireframeMode ? gPipelineStates.mWireframeRS : nullptr);

    CameraUtils::updateViewMatrix(mCamera);
       
    drawGrid(direct3DData);

    // Present results
    const HRESULT result = direct3DData.mSwapChain->Present(0, 0);
    DxErrorChecker(result);
}

void GPUAcceleratedInterlockingTilesAlgorithmNaive::onMouseMove(WPARAM btnState, 
                                                                const int32_t x, 
                                                                const int32_t y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        const float dx = DirectX::XMConvertToRadians(0.15f * static_cast<float>(x - mLastMousePosition.x));
        const float dy = DirectX::XMConvertToRadians(0.15f * static_cast<float>(y - mLastMousePosition.y));

        CameraUtils::pitch(dy, mCamera);
        CameraUtils::rotateY(dx, mCamera);
    }

    mLastMousePosition.x = x;
    mLastMousePosition.y = y;
}

void GPUAcceleratedInterlockingTilesAlgorithmNaive::drawGrid(Direct3DData& direct3DData)
{
    //
    // Input Assembler Stage
    //

    // Input Layout 
    ID3D11InputLayout * const inputLayout = gShaders.mTerrainIL;
    direct3DData.mImmediateContext->IASetInputLayout(inputLayout);

    // Primitive Topology
    direct3DData.mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST);

    // Vertex Buffer
    ID3D11Buffer* vertexBuffer = gGeometryBuffers.mBufferInfo->mVertexBuffer;
    uint32_t stride = sizeof(Vertex);
    uint32_t offset = 0;
    direct3DData.mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // Index Buffer
    ID3D11Buffer* indexBuffer = gGeometryBuffers.mBufferInfo->mIndexBuffer;
    direct3DData.mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //
    // Vertex Shader Stage
    //
        
    // Shader
    ID3D11VertexShader * const vertexShader = gShaders.mTerrainVS;
    direct3DData.mImmediateContext->VSSetShader(vertexShader, nullptr, 0);

    // Per Frame Constant Buffer
    const DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mWorldMatrix);
    const DirectX::XMMATRIX textureScale = DirectX::XMLoadFloat4x4(&mTextureScaleMatrix);
    DirectX::XMStoreFloat4x4(&mGridVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));
    DirectX::XMStoreFloat4x4(&mGridVSPerObjectBuffer.mData.mTextureScale, DirectX::XMMatrixTranspose(textureScale));
    ConstantBufferUtils::applyChanges(*direct3DData.mImmediateContext, mGridVSPerObjectBuffer);

    // Set Constant Buffers
    ID3D11Buffer* const vertexShaderBuffers[] = { mGridVSPerObjectBuffer.mBuffer };
    direct3DData.mImmediateContext->VSSetConstantBuffers(0, 1, vertexShaderBuffers);

    //
    // Hull Shader Stage
    //

    // Shader
    ID3D11HullShader * const hullShader = gShaders.mTerrainHS;
    direct3DData.mImmediateContext->HSSetShader(hullShader, nullptr, 0);

    // Per Frame Constant Buffer
    mGridHSPerFrameBuffer.mData.mEyePositionW = mCamera.mPosition;
    ConstantBufferUtils::applyChanges(*direct3DData.mImmediateContext, mGridHSPerFrameBuffer);

    // Set Constant Buffers
    ID3D11Buffer* const hullShaderBuffers = { mGridHSPerFrameBuffer.mBuffer };
    direct3DData.mImmediateContext->HSSetConstantBuffers(0, 1, &hullShaderBuffers);

    //
    // Domain Shader Stage
    //

    // Shader
    ID3D11DomainShader * const domainShader = gShaders.mTerrainDS;
    direct3DData.mImmediateContext->DSSetShader(domainShader, nullptr, 0);

    // Per Frame Constant Buffer
    const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);
    const DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
    DirectX::XMStoreFloat4x4(&mGridDSPerFrameBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));
    DirectX::XMStoreFloat4x4(&mGridDSPerFrameBuffer.mData.mViewProjection, DirectX::XMMatrixTranspose(viewProjection));
    const float heightMapTexelSize = 1.0f / 512.0f;
    mGridDSPerFrameBuffer.mData.mHeightMapTexelWidthHeight[0] = heightMapTexelSize;
    mGridDSPerFrameBuffer.mData.mHeightMapTexelWidthHeight[1] = heightMapTexelSize;
    ConstantBufferUtils::applyChanges(*direct3DData.mImmediateContext, mGridDSPerFrameBuffer);

    // Set Constant Buffers
    ID3D11Buffer* const domainShaderBuffers = { mGridDSPerFrameBuffer.mBuffer };
    direct3DData.mImmediateContext->DSSetConstantBuffers(0, 1, &domainShaderBuffers);

    // Resources
    ID3D11ShaderResourceView * const domainShaderResources[] = { gShaderResources.mHeightMapSRV };
    direct3DData.mImmediateContext->DSSetShaderResources(0, 1, domainShaderResources);

    // Sampler state
    direct3DData.mImmediateContext->DSSetSamplers(0, 1, &gPipelineStates.mLinearSS);

    //
    // Pixel Shader Stage
    //

    // Shader
    ID3D11PixelShader* const pixelShader = gShaders.mTerrainPS;
    direct3DData.mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

    // Per Frame Constant Buffer
    memcpy(&mGridPSPerFrameBuffer.mData.mDirectionalLight, &mDirectionalLight, sizeof(mDirectionalLight));
    mGridPSPerFrameBuffer.mData.mEyePositionW = mCamera.mPosition;
    mGridPSPerFrameBuffer.mData.mMaterial = mTerrainMaterial;
    mGridPSPerFrameBuffer.mData.mTexelCellSpaceU = heightMapTexelSize;
    mGridPSPerFrameBuffer.mData.mTexelCellSpaceV = heightMapTexelSize;
    mGridPSPerFrameBuffer.mData.mWorldCellSpace = 0.5f;
    ConstantBufferUtils::applyChanges(*direct3DData.mImmediateContext, mGridPSPerFrameBuffer);

    // Set constant buffers
    ID3D11Buffer * const pixelShaderBuffers[] = { mGridPSPerFrameBuffer.mBuffer };
    direct3DData.mImmediateContext->PSSetConstantBuffers(0, 1, pixelShaderBuffers);
                
    // Resources
    ID3D11ShaderResourceView * const pixelShaderResources[] = 
    { 
        gShaderResources.mTerrainDiffuseMapSRV,
        gShaderResources.mHeightMapSRV
    };
    direct3DData.mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

    // Sampler state
    direct3DData.mImmediateContext->PSSetSamplers(0, 1, &gPipelineStates.mLinearSS);

    //
    // Draw
    // 
    const uint32_t baseVertexLocation = gGeometryBuffers.mBufferInfo->mBaseVertexLocation;
    const uint32_t startIndexLocation = gGeometryBuffers.mBufferInfo->mStartIndexLocation;
    const uint32_t indexCount = gGeometryBuffers.mBufferInfo->mIndexCount;
    direct3DData.mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}