//////////////////////////////////////////////////////////////////////////
// Demonstrates texturing an scene.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <limits>
#include <vector>

#include "Waves/Waves.h"
#include "HLSL/Buffers.h"

#include <ConstantBuffer.h>
#include <D3DApplication.h>
#include <DDSTextureLoader.h>
#include <DxErrorChecker.h> 
#include <LightHelper.h>
#include <MathHelper.h>

namespace Framework
{
    class BlendingApp : public D3DApplication
    {
    public:
        enum struct RenderOptions
        {
            LIGHTING,
            LIGHTING_TEXTURING,
            LIGHTING_TEXTURING_FOG
        };
    public:
        inline BlendingApp(HINSTANCE hInstance);

        inline ~BlendingApp();

        inline bool init();

        inline void onResize();

        void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:
        void buildGeometryBuffers();
        
        void buildShaders();

        inline void buildVertexLayout(std::vector<char>& compiledShader);

        inline void buildSamplerStates();

        inline void buildBlendStates();

        inline void loadTextures();

    private:
        Geometry::Waves mWaves;

        DirectionalLight mDirectionalLight;
        PointLight mPointLight;
        SpotLight mSpotLight;
        Material mLandMaterial;
        Material mWavesMaterial;

        ID3D11Buffer* mLandVertexBuffer;
        ID3D11Buffer* mWavesVertexBuffer;
        ID3D11Buffer* mIndexBuffer;

        ID3D11VertexShader* mVertexShader;
        ID3D11PixelShader* mLightingPixelShader;
        ID3D11PixelShader* mLightingTexturingPixelShader;
        ID3D11PixelShader* mLightingTexturingFogPixelShader;

        ID3D11ShaderResourceView* mGrassTextureSRV;
        ID3D11ShaderResourceView* mWaterTextureSRV;
        
        ID3D11InputLayout* mInputLayout;

        ID3D11SamplerState* mSamplerState;
        ID3D11BlendState* mTransparentBS;

        Shaders::ConstantBuffer<Shaders::ImmutableBuffer> mImmutableBuffer;
        Shaders::ConstantBuffer<Shaders::PerFrameBuffer> mPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::PerObjectBuffer> mPerObjectBuffer;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mLandWorld;
        DirectX::XMFLOAT4X4 mWavesWorld;

        // Define textures transformations
        DirectX::XMFLOAT4X4 mGrassTexTransform;
        DirectX::XMFLOAT4X4 mWaterTexTransform;

        DirectX::XMFLOAT4X4 mView;
        DirectX::XMFLOAT4X4 mProjection;

        DirectX::XMVECTORF32 mClearScreenColor;

        DirectX::XMFLOAT3 mEyePositionW;
        
        DirectX::XMFLOAT2 mWaterTexOffset;

        uint32_t mLandIndexOffset;
        uint32_t mWavesIndexOffset;
        
        uint32_t mLandIndexCount;
        uint32_t mWavesIndexCount;
        
        float mTheta;
        float mPhi;
        float mRadius;

        POINT mLastMousePos;

        RenderOptions mRenderOptions;
    };     

    inline BlendingApp::BlendingApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mLandVertexBuffer(nullptr)
        , mWavesVertexBuffer(nullptr)
        , mIndexBuffer(nullptr)
        , mVertexShader(nullptr)
        , mLightingPixelShader(nullptr)
        , mLightingTexturingPixelShader(nullptr)
        , mLightingTexturingFogPixelShader(nullptr)
        , mInputLayout(nullptr)
        , mGrassTextureSRV(nullptr)
        , mWaterTextureSRV(nullptr)
        , mSamplerState(nullptr)
        , mTransparentBS(nullptr)
        , mClearScreenColor(DirectX::Colors::LightSkyBlue)
        , mEyePositionW(0.0f, 0.0f, 0.0f)
        , mWaterTexOffset(0.0f, 0.0f)
        , mLandIndexOffset(0)
        , mWavesIndexOffset(0)            
        , mLandIndexCount(0)
        , mWavesIndexCount(0)
        , mTheta(1.5f * DirectX::XM_PI)
        , mPhi(0.25f * DirectX::XM_PI)
        , mRadius(5.0f)
        , mRenderOptions(RenderOptions::LIGHTING)
    {
        mMainWindowCaption = L"Waves Demo";

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mLandWorld, I);
        DirectX::XMStoreFloat4x4(&mWavesWorld, I);
        DirectX::XMStoreFloat4x4(&mView, I);
        DirectX::XMStoreFloat4x4(&mProjection, I);  

        DirectX::XMMATRIX wavesOffset = DirectX::XMMatrixTranslation(0.0f, -3.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mWavesWorld, wavesOffset);

        // Texture transformation matrices
        DirectX::XMMATRIX grassTexScale = DirectX::XMMatrixScaling(5.0f, 5.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);
        DirectX::XMStoreFloat4x4(&mWaterTexTransform, DirectX::XMMatrixIdentity());

        // Directional light.
        mDirectionalLight.mAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight.mDiffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mDirectionalLight.mSpecular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mDirectionalLight.mDirection = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

        // Point light--position is changed every frame to animate in UpdateScene function.
        mPointLight.mAmbient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
        mPointLight.mDiffuse = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
        mPointLight.mSpecular = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
        mPointLight.mAttenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
        mPointLight.mRange = 25.0f;

        // Spot light--position and direction changed every frame to animate in UpdateScene function.
        mSpotLight.mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        mSpotLight.mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
        mSpotLight.mSpecular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mSpotLight.mAttenuation = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
        mSpotLight.mSpot = 96.0f;
        mSpotLight.mRange = 10000.0f;

        // Initialize materials
        mLandMaterial.mAmbient = DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        mLandMaterial.mDiffuse = DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        mLandMaterial.mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

        mWavesMaterial.mAmbient = DirectX::XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
        mWavesMaterial.mDiffuse = DirectX::XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
        mWavesMaterial.mSpecular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
    }

    inline BlendingApp::~BlendingApp()
    {
        mLandVertexBuffer->Release();
        mWavesVertexBuffer->Release();
        mIndexBuffer->Release();
        mVertexShader->Release();
        mLightingPixelShader->Release();
        mLightingTexturingPixelShader->Release();
        mLightingTexturingFogPixelShader->Release();
        mGrassTextureSRV->Release();
        mWaterTextureSRV->Release();
        mInputLayout->Release();
        mSamplerState->Release();
        mTransparentBS->Release();
    }

    inline bool BlendingApp::init()
    {
        if(!D3DApplication::init())
            return false;

        mWaves.init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

        buildGeometryBuffers();
        buildShaders();       

        // Initialize immutable constant buffer
        mImmutableBuffer.initialize(*mDevice);
        mImmutableBuffer.mData.mFogStart = 15.0f;
        mImmutableBuffer.mData.mFogRange = 175.0f;
        DirectX::XMStoreFloat4(&mImmutableBuffer.mData.mFogColor, DirectX::Colors::Silver);
        mImmutableBuffer.applyChanges(*mImmediateContext);

        mPerFrameBuffer.initialize(*mDevice);
        mPerObjectBuffer.initialize(*mDevice);
        buildSamplerStates();
        buildBlendStates();
        loadTextures();

        return true;
    }

    inline void BlendingApp::onResize()
    {
        D3DApplication::onResize();

        // The window resized, so update the aspect ratio and recompute the projection matrix.
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&mProjection, P);
    }

    inline void BlendingApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void BlendingApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }   

    inline void BlendingApp::buildVertexLayout(std::vector<char>& compiledShader)
    {
        // Create the vertex input layout.
        D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        // Create the input layout
        const HRESULT result = mDevice->CreateInputLayout(vertexDesc, 3, &compiledShader[0], 
            compiledShader.size(), &mInputLayout);

        DxErrorChecker(result);
    }

    inline void BlendingApp::buildSamplerStates()
    {
        D3D11_SAMPLER_DESC samplerDesc; 
        samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; 
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; 
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; 
        samplerDesc.MipLODBias = 0; 
        samplerDesc.MaxAnisotropy = 4; 
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; 
        samplerDesc.BorderColor[0] = 1.0f; 
        samplerDesc.BorderColor[1] = 1.0f; 
        samplerDesc.BorderColor[2] = 1.0f; 
        samplerDesc.BorderColor[3] = 1.0f; 
        samplerDesc.MinLOD = -3.402823466e+38F; // FLT_MAX 
        samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MIN 

        HRESULT result = mDevice->CreateSamplerState(&samplerDesc, &mSamplerState);
        DxErrorChecker(result);
    }

    inline void BlendingApp::buildBlendStates()
    {
        D3D11_BLEND_DESC transparentDesc = {0};
        transparentDesc.AlphaToCoverageEnable = false;
        transparentDesc.IndependentBlendEnable = false;

        transparentDesc.RenderTarget[0].BlendEnable = true;
        transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        HRESULT result = mDevice->CreateBlendState(&transparentDesc, &mTransparentBS);
        DxErrorChecker(result);
    }

    inline void BlendingApp::loadTextures()
    {
        ID3D11Resource* texture = nullptr;

        // Create grass texture shader resource view
        HRESULT result = CreateDDSTextureFromFile(mDevice, L"Resources/Textures/grass.dds", &texture, &mGrassTextureSRV);
        DxErrorChecker(result);        
        texture->Release();

        // Create water texture shader resource view.
        result = CreateDDSTextureFromFile(mDevice, L"Resources/Textures/water1.dds", &texture, &mWaterTextureSRV);
        DxErrorChecker(result);        
        texture->Release();
    }
}