#include "GPUAcceleratedInterlockingTilesAlgorithmNaive.h"

#include <sstream>
#include <WindowsX.h>

#include <ConstantBuffer.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <DxErrorChecker.h>
#include <GeometryGenerator.h>
#include <MathHelper.h>

#include "Globals.h"

namespace
{
    void calculateFrameStats()
    {
        // Code computes the average frames per second, and also the 
        // average time it takes to render one frame.  These stats 
        // are appended to the window caption bar.
        static uint32_t frameCounter = 0;
        static float timeElapsed = 0.0f;

        ++frameCounter;

        // Compute averages over one second period.
        if( (TimerUtils::inGameTime(Globals::gTimer) - timeElapsed) >= 1.0f )
        {
            const float fps = static_cast<float> (frameCounter); // fps = frameCnt / 1
            const float mspf = 1000.0f / fps;

            std::wostringstream outs;   
            outs.precision(6);
            outs << L"GPU Accelerated Interlocking Tiles Algorithm Naive Demo" << L"    "
                << L"FPS: " << fps << L"    " 
                << L"Frame Time: " << mspf << L" (ms)";

            SetWindowText(Globals::gWindowData.mMainWindow, outs.str().c_str());

            // Reset for next average.
            frameCounter = 0;
            timeElapsed += 1.0f;
        }
    }
}

namespace TerrainSceneUtils
{
    void init(TerrainScene& terrainScene)
    {
        // Directional lights.
        terrainScene.mDirectionalLight[0].mAmbient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
        terrainScene.mDirectionalLight[0].mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        terrainScene.mDirectionalLight[0].mSpecular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
        terrainScene.mDirectionalLight[0].mDirection = DirectX::XMFLOAT3(0.707f, -0.707f, 0.0f);

        terrainScene.mDirectionalLight[1].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        terrainScene.mDirectionalLight[1].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        terrainScene.mDirectionalLight[1].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        terrainScene.mDirectionalLight[1].mDirection = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

        terrainScene.mDirectionalLight[2].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        terrainScene.mDirectionalLight[2].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        terrainScene.mDirectionalLight[2].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        terrainScene.mDirectionalLight[2].mDirection = DirectX::XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);

        // Initialize material
        terrainScene.mTerrainMaterial.mAmbient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        terrainScene.mTerrainMaterial.mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        terrainScene.mTerrainMaterial.mSpecular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
        terrainScene.mTerrainMaterial.mReflect = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    
        // Initialize constant buffers
        assert(Globals::gDirect3DData.mDevice);
        ConstantBufferUtils::initialize(*Globals::gDirect3DData.mDevice, terrainScene.mGridPSPerFrameBuffer);
        ConstantBufferUtils::initialize(*Globals::gDirect3DData.mDevice, terrainScene.mGridHSPerFrameBuffer);
        ConstantBufferUtils::initialize(*Globals::gDirect3DData.mDevice, terrainScene.mGridDSPerFrameBuffer);
        ConstantBufferUtils::initialize(*Globals::gDirect3DData.mDevice, terrainScene.mGridVSPerObjectBuffer);
        
        // Terrain world matrix
        const DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&terrainScene.mWorldMatrix, translation);

        // Texture scale matrix
        const DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 1.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&terrainScene.mTextureScaleMatrix, scale);
    }

    void draw(TerrainScene& terrainScene)
    {
        //
        // Input Assembler Stage
        //

        // Input Layout 
        ID3D11InputLayout * const inputLayout = Globals::gShaders.mTerrainIL;
        Globals::gDirect3DData.mImmediateContext->IASetInputLayout(inputLayout);

        // Primitive Topology
        Globals::gDirect3DData.mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST);

        // Vertex Buffer
        ID3D11Buffer* vertexBuffer = Globals::gGeometryBuffers.mBufferInfo->mVertexBuffer;
        uint32_t stride = sizeof(Vertex);
        uint32_t offset = 0;
        Globals::gDirect3DData.mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        // Index Buffer
        ID3D11Buffer* indexBuffer = Globals::gGeometryBuffers.mBufferInfo->mIndexBuffer;
        Globals::gDirect3DData.mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        //
        // Vertex Shader Stage
        //

        // Shader
        ID3D11VertexShader * const vertexShader = Globals::gShaders.mTerrainVS;
        Globals::gDirect3DData.mImmediateContext->VSSetShader(vertexShader, nullptr, 0);

        // Per Frame Constant Buffer
        const DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&terrainScene.mWorldMatrix);
        const DirectX::XMMATRIX textureScale = DirectX::XMLoadFloat4x4(&terrainScene.mTextureScaleMatrix);
        DirectX::XMStoreFloat4x4(&terrainScene.mGridVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));
        DirectX::XMStoreFloat4x4(&terrainScene.mGridVSPerObjectBuffer.mData.mTextureScale, DirectX::XMMatrixTranspose(textureScale));
        ConstantBufferUtils::applyChanges(*Globals::gDirect3DData.mImmediateContext, terrainScene.mGridVSPerObjectBuffer);

        // Set Constant Buffers
        ID3D11Buffer* const vertexShaderBuffers[] = { terrainScene.mGridVSPerObjectBuffer.mBuffer };
        Globals::gDirect3DData.mImmediateContext->VSSetConstantBuffers(0, 1, vertexShaderBuffers);

        //
        // Hull Shader Stage
        //

        // Shader
        ID3D11HullShader * const hullShader = Globals::gShaders.mTerrainHS;
        Globals::gDirect3DData.mImmediateContext->HSSetShader(hullShader, nullptr, 0);

        // Per Frame Constant Buffer
        terrainScene.mGridHSPerFrameBuffer.mData.mEyePositionW = Globals::gCamera.mPosition;
        ConstantBufferUtils::applyChanges(*Globals::gDirect3DData.mImmediateContext, terrainScene.mGridHSPerFrameBuffer);

        // Set Constant Buffers
        ID3D11Buffer* const hullShaderBuffers = { terrainScene.mGridHSPerFrameBuffer.mBuffer };
        Globals::gDirect3DData.mImmediateContext->HSSetConstantBuffers(0, 1, &hullShaderBuffers);

        //
        // Domain Shader Stage
        //

        // Shader
        ID3D11DomainShader * const domainShader = Globals::gShaders.mTerrainDS;
        Globals::gDirect3DData.mImmediateContext->DSSetShader(domainShader, nullptr, 0);

        // Per Frame Constant Buffer
        const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(Globals::gCamera);
        const DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
        DirectX::XMStoreFloat4x4(&terrainScene.mGridDSPerFrameBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));
        DirectX::XMStoreFloat4x4(&terrainScene.mGridDSPerFrameBuffer.mData.mViewProjection, DirectX::XMMatrixTranspose(viewProjection));
        const float heightMapTexelSize = 1.0f / 512.0f;
        terrainScene.mGridDSPerFrameBuffer.mData.mHeightMapTexelWidthHeight[0] = heightMapTexelSize;
        terrainScene.mGridDSPerFrameBuffer.mData.mHeightMapTexelWidthHeight[1] = heightMapTexelSize;
        ConstantBufferUtils::applyChanges(*Globals::gDirect3DData.mImmediateContext, terrainScene.mGridDSPerFrameBuffer);

        // Set Constant Buffers
        ID3D11Buffer* const domainShaderBuffers = { terrainScene.mGridDSPerFrameBuffer.mBuffer };
        Globals::gDirect3DData.mImmediateContext->DSSetConstantBuffers(0, 1, &domainShaderBuffers);

        // Resources
        ID3D11ShaderResourceView * const domainShaderResources[] = { Globals::gShaderResources.mHeightMapSRV };
        Globals::gDirect3DData.mImmediateContext->DSSetShaderResources(0, 1, domainShaderResources);

        // Sampler state
        Globals::gDirect3DData.mImmediateContext->DSSetSamplers(0, 1, &Globals::gPipelineStates.mLinearSS);

        //
        // Pixel Shader Stage
        //

        // Shader
        ID3D11PixelShader* const pixelShader = Globals::gShaders.mTerrainPS;
        Globals::gDirect3DData.mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        // Per Frame Constant Buffer
        memcpy(&terrainScene.mGridPSPerFrameBuffer.mData.mDirectionalLight, &terrainScene.mDirectionalLight, sizeof(terrainScene.mDirectionalLight));
        terrainScene.mGridPSPerFrameBuffer.mData.mEyePositionW = Globals::gCamera.mPosition;
        terrainScene.mGridPSPerFrameBuffer.mData.mMaterial = terrainScene.mTerrainMaterial;
        terrainScene.mGridPSPerFrameBuffer.mData.mTexelCellSpaceU = heightMapTexelSize;
        terrainScene.mGridPSPerFrameBuffer.mData.mTexelCellSpaceV = heightMapTexelSize;
        terrainScene.mGridPSPerFrameBuffer.mData.mWorldCellSpace = 0.5f;
        ConstantBufferUtils::applyChanges(*Globals::gDirect3DData.mImmediateContext, terrainScene.mGridPSPerFrameBuffer);

        // Set constant buffers
        ID3D11Buffer * const pixelShaderBuffers[] = { terrainScene.mGridPSPerFrameBuffer.mBuffer };
        Globals::gDirect3DData.mImmediateContext->PSSetConstantBuffers(0, 1, pixelShaderBuffers);

        // Resources
        ID3D11ShaderResourceView * const pixelShaderResources[] = 
        { 
            Globals::gShaderResources.mTerrainDiffuseMapSRV,
            Globals::gShaderResources.mHeightMapSRV
        };
        Globals::gDirect3DData.mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        // Sampler state
        Globals::gDirect3DData.mImmediateContext->PSSetSamplers(0, 1, &Globals::gPipelineStates.mLinearSS);

        //
        // Draw
        // 
        const uint32_t baseVertexLocation = Globals::gGeometryBuffers.mBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Globals::gGeometryBuffers.mBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Globals::gGeometryBuffers.mBufferInfo->mIndexCount;
        Globals::gDirect3DData.mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }
}

GPUAcceleratedInterlockingTilesAlgorithmNaive::GPUAcceleratedInterlockingTilesAlgorithmNaive()
{
   
}

GPUAcceleratedInterlockingTilesAlgorithmNaive::~GPUAcceleratedInterlockingTilesAlgorithmNaive()
{
    GlobalsUtils::destroy();
}

bool GPUAcceleratedInterlockingTilesAlgorithmNaive::init()
{
    GlobalsUtils::init();

    TerrainSceneUtils::init(gTerrainScene);
    
    Globals::gCamera.mPosition = DirectX::XMFLOAT3(0.0f, 2.0f, -15.0f);

    return true;
}

void updateScene(const float dt)
{
    //
    // Control the camera.
    //
    const float offset = 50.0f;
    if (GetAsyncKeyState('W') & 0x8000)
    {
        CameraUtils::walk(offset * dt, Globals::gCamera);
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        CameraUtils::walk(-offset * dt, Globals::gCamera);
    }

    if (GetAsyncKeyState('A') & 0x8000)
    {
        CameraUtils::strafe(-offset * dt, Globals::gCamera);
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        CameraUtils::strafe(offset * dt, Globals::gCamera);
    }

    if (GetAsyncKeyState('T') & 0x8000) 
    {
        Globals::gWindowData.mWireframeMode = true;
    }

    if (GetAsyncKeyState('Y') & 0x8000) 
    {
        Globals::gWindowData.mWireframeMode = false;
    }
}

int run()
{
    MSG msg = {0};

    TimerUtils::reset(Globals::gTimer);

    while (msg.message != WM_QUIT)
    {
        // If there are Window messages then process them.
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Otherwise, do animation/game stuff.
        else
        {	
            TimerUtils::tick(Globals::gTimer);

            if (!Globals::gWindowState.mIsPaused)
            {
                calculateFrameStats();
                updateScene(static_cast<float> (Globals::gTimer.mDeltaTime));	
                drawScene();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return static_cast<int> (msg.wParam);
}

void drawScene()
{
    assert(Globals::gDirect3DData.mImmediateContext);

    Globals::gDirect3DData.mImmediateContext->ClearRenderTargetView(Globals::gDirect3DData.mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Silver));
    Globals::gDirect3DData.mImmediateContext->ClearDepthStencilView(Globals::gDirect3DData.mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    Globals::gDirect3DData.mImmediateContext->RSSetState(Globals::gWindowData.mWireframeMode ? Globals::gPipelineStates.mWireframeRS : nullptr);

    CameraUtils::updateViewMatrix(Globals::gCamera);
       
    TerrainSceneUtils::draw(gTerrainScene);

    // Present results
    const HRESULT result = Globals::gDirect3DData.mSwapChain->Present(0, 0);
    DxErrorChecker(result);
}