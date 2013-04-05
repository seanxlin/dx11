//////////////////////////////////////////////////////////////////////////
// Demonstrates 3D lighting with directional, point, and spot lights.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <vector>

#include "Waves/Waves.h"
#include "HLSL/Buffers.h"

#include <ConstantBuffer.h>
#include <D3DApplication.h>
#include <DxErrorChecker.h>
#include <LightHelper.h>
#include <MathHelper.h>

namespace Framework
{
    class LightingApp : public D3DApplication
    {
    public:
        inline LightingApp(HINSTANCE hInstance);

        inline ~LightingApp();

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
        ID3D11PixelShader* mPixelShader;

        Shaders::ConstantBuffer<Shaders::PerFrameBuffer> mPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::PerObjectBuffer> mPerObjectBuffer;

        ID3D11InputLayout* mInputLayout;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mLandWorld;
        DirectX::XMFLOAT4X4 mWavesWorld;

        DirectX::XMFLOAT4X4 mView;
        DirectX::XMFLOAT4X4 mProjection;

        DirectX::XMFLOAT3 mEyePositionW;

        uint32_t mLandIndexOffset;
        uint32_t mWavesIndexOffset;
        
        uint32_t mLandIndexCount;
        uint32_t mWavesIndexCount;
        
        float mTheta;
        float mPhi;
        float mRadius;
        
        POINT mLastMousePos;
    };     

    inline LightingApp::LightingApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mLandVertexBuffer(nullptr)
        , mWavesVertexBuffer(nullptr)
        , mIndexBuffer(nullptr)
        , mVertexShader(nullptr)
        , mPixelShader(nullptr)
        , mInputLayout(nullptr)
        , mEyePositionW(0.0f, 0.0f, 0.0f)
        , mLandIndexOffset(0)
        , mWavesIndexOffset(0)            
        , mLandIndexCount(0)
        , mWavesIndexCount(0)
        , mTheta(1.5f * DirectX::XM_PI)
        , mPhi(0.25f * DirectX::XM_PI)
        , mRadius(5.0f)
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

        mLandMaterial.mAmbient = DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        mLandMaterial.mDiffuse = DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        mLandMaterial.mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

        mWavesMaterial.mAmbient = DirectX::XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
        mWavesMaterial.mDiffuse = DirectX::XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
        mWavesMaterial.mSpecular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
    }

    inline LightingApp::~LightingApp()
    {
        mLandVertexBuffer->Release();
        mWavesVertexBuffer->Release();
        mIndexBuffer->Release();
        mVertexShader->Release();
        mPixelShader->Release();
        mInputLayout->Release();
    }

    inline bool LightingApp::init()
    {
        if(!D3DApplication::init())
            return false;

        mWaves.init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

        buildGeometryBuffers();
        buildShaders();       
        mPerFrameBuffer.initialize(*mDevice);
        mPerObjectBuffer.initialize(*mDevice);

        return true;
    }

    inline void LightingApp::onResize()
    {
        D3DApplication::onResize();

        // The window resized, so update the aspect ratio and recompute the projection matrix.
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&mProjection, P);
    }

    inline void LightingApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void LightingApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }   

    inline void LightingApp::buildVertexLayout(std::vector<char>& compiledShader)
    {
        // Create the vertex input layout.
        D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        // Create the input layout
        const HRESULT result = mDevice->CreateInputLayout(vertexDesc, 2, &compiledShader[0], 
            compiledShader.size(), &mInputLayout);

        DxErrorChecker(result);
    }
}