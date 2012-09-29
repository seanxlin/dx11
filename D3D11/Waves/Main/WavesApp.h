//////////////////////////////////////////////////////////////////////////
// Demostrates rendering of a land and waves. The land has color
// and the waves will be drawn in wireframe mode.
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
#include <MathHelper.h>

namespace Framework
{
    class WavesApp : public D3DApplication
    {
    public:
        inline WavesApp(HINSTANCE hInstance);

        inline ~WavesApp();

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

        inline void buildRasterizerState();

    private:
        Geometry::Waves mWaves;

        ID3D11Buffer* mLandVertexBuffer;
        ID3D11Buffer* mWavesVertexBuffer;
        ID3D11Buffer* mIndexBuffer;

        ID3D11VertexShader* mVertexShader;
        ID3D11PixelShader* mPixelShader;

        Shaders::ConstantBuffer<Shaders::PerObjectBuffer> mPerObjectBuffer;

        ID3D11InputLayout* mInputLayout;
        
        ID3D11RasterizerState* mWireframeRS;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mLandWorld;
        DirectX::XMFLOAT4X4 mWavesWorld;

        DirectX::XMFLOAT4X4 mView;
        DirectX::XMFLOAT4X4 mProjection;

        uint32_t mLandIndexOffset;
        uint32_t mWavesIndexOffset;
        
        uint32_t mLandIndexCount;
        uint32_t mWavesIndexCount;
        
        float mTheta;
        float mPhi;
        float mRadius;
        
        POINT mLastMousePos;
    };     

    inline WavesApp::WavesApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mLandVertexBuffer(nullptr)
        , mWavesVertexBuffer(nullptr)
        , mIndexBuffer(nullptr)
        , mVertexShader(nullptr)
        , mPixelShader(nullptr)
        , mInputLayout(nullptr)
        , mWireframeRS(nullptr)
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
    }

    inline WavesApp::~WavesApp()
    {
        mLandVertexBuffer->Release();
        mWavesVertexBuffer->Release();
        mIndexBuffer->Release();
        mVertexShader->Release();
        mPixelShader->Release();
        mInputLayout->Release();
        mWireframeRS->Release();
    }

    inline bool WavesApp::init()
    {
        if(!D3DApplication::init())
            return false;

        mWaves.init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

        buildGeometryBuffers();
        buildShaders();            
        buildRasterizerState();
        mPerObjectBuffer.initialize(mDevice);

        return true;
    }

    inline void WavesApp::onResize()
    {
        D3DApplication::onResize();

        // The window resized, so update the aspect ratio and recompute the projection matrix.
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&mProjection, P);
    }

    inline void WavesApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void WavesApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }   

    inline void WavesApp::buildVertexLayout(std::vector<char>& compiledShader)
    {
        // Create the vertex input layout.
        D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        // Create the input layout
        const HRESULT result = mDevice->CreateInputLayout(vertexDesc, 2, &compiledShader[0], 
            compiledShader.size(), &mInputLayout);

        DebugUtils::DxErrorChecker(result);
    }

    inline void WavesApp::buildRasterizerState()
    {
        D3D11_RASTERIZER_DESC wireframeDesc;
        ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
        wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
        wireframeDesc.CullMode = D3D11_CULL_BACK;
        wireframeDesc.FrontCounterClockwise = false;
        wireframeDesc.DepthClipEnable = true;

        HRESULT result = mDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS);
        DebugUtils::DxErrorChecker(result);
    }
}