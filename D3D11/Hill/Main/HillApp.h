//////////////////////////////////////////////////////////////////////////
// Demonstrates rendering a hill.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <vector>

#include "HLSL/Buffers.h"

#include <ConstantBuffer.h>
#include <D3DApplication.h>
#include <D3DErrorChecker.h>

namespace Framework
{
    class HillApp : public D3DApplication
    {
    public:
        inline HillApp(HINSTANCE hInstance);

        inline ~HillApp();

        inline bool init();

        inline void onResize();

        inline void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:
        void buildGeometryBuffers();
        void buildShaders();

        inline void buildVertexLayout(std::vector<char>& compiledShader);

    private:
        ID3D11Buffer* mGridVertexBuffer;
        ID3D11Buffer* mGridIndexBuffer;

        ID3D11VertexShader* mVertexShader;
        ID3D11PixelShader* mPixelShader;

        Shaders::ConstantBuffer<Shaders::PerObjectBuffer> mPerFrameBuffer;

        ID3D11InputLayout* mInputLayout;

        DirectX::XMFLOAT4X4 mWorld;
        DirectX::XMFLOAT4X4 mView;
        DirectX::XMFLOAT4X4 mProjection;

        float mTheta;
        float mPhi;
        float mRadius;

        uint64_t mGridIndexCount;
        
        POINT mLastMousePos;
    };     

    inline HillApp::HillApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mGridVertexBuffer(nullptr)
        , mGridIndexBuffer(nullptr)
        , mVertexShader(nullptr)
        , mPixelShader(nullptr)
        , mInputLayout(nullptr)
        , mTheta(1.5f * DirectX::XM_PI)
        , mPhi(0.25f * DirectX::XM_PI)
        , mRadius(5.0f)
        , mGridIndexCount(0)
    {
        mMainWindowCaption = L"Hill Demo";

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mWorld, I);
        DirectX::XMStoreFloat4x4(&mView, I);
        DirectX::XMStoreFloat4x4(&mProjection, I);
    }

    inline HillApp::~HillApp()
    {
        mGridVertexBuffer->Release();
        mGridIndexBuffer->Release();
        mVertexShader->Release();
        mPixelShader->Release();
        mInputLayout->Release();
    }

    inline bool HillApp::init()
    {
        if(!D3DApplication::init())
            return false;

        buildGeometryBuffers();
        buildShaders();            
        mPerFrameBuffer.initialize(mDevice);

        return true;
    }

    inline void HillApp::onResize()
    {
        D3DApplication::onResize();

        // The window resized, so update the aspect ratio and recompute the projection matrix.
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&mProjection, P);
    }

    inline void HillApp::updateScene(const float dt)
    {
        // Convert Spherical to Cartesian coordinates.
        const float x = mRadius * sinf(mPhi) * cosf(mTheta);
        const float z = mRadius * sinf(mPhi) * sinf(mTheta);
        const float y = mRadius * cosf(mPhi);

        // Build the view matrix.
        DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
        DirectX::XMVECTOR target = DirectX::XMVectorZero();
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(pos, target, up);
        XMStoreFloat4x4(&mView, viewMatrix);
    }

    inline void HillApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void HillApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }


    inline void HillApp::buildVertexLayout(std::vector<char>& compiledShader)
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

        DebugUtils::ErrorChecker(result);
    }
}