//////////////////////////////////////////////////////////////////////////
// Demonstrates rendering a colored box.
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
    class BoxApp : public D3DApplication
    {
    public:
        inline BoxApp(HINSTANCE hInstance);

        inline ~BoxApp();

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
        ID3D11Buffer* mBoxVertexBuffer;
        ID3D11Buffer* mBoxIndexBuffer;

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

        POINT mLastMousePos;
    };     

    inline BoxApp::BoxApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mBoxVertexBuffer(nullptr)
        , mBoxIndexBuffer(nullptr)
        , mVertexShader(nullptr)
        , mPixelShader(nullptr)
        , mInputLayout(nullptr)
        , mTheta(1.5f * DirectX::XM_PI)
        , mPhi(0.25f * DirectX::XM_PI)
        , mRadius(5.0f)
    {
        mMainWindowCaption = L"Box Demo";

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mWorld, I);
        DirectX::XMStoreFloat4x4(&mView, I);
        DirectX::XMStoreFloat4x4(&mProjection, I);
    }

    inline BoxApp::~BoxApp()
    {
        mBoxVertexBuffer->Release();
        mBoxIndexBuffer->Release();
        mVertexShader->Release();
        mPixelShader->Release();
        mInputLayout->Release();
    }

    inline bool BoxApp::init()
    {
        if(!D3DApplication::init())
            return false;

        buildGeometryBuffers();
        buildShaders();            
        mPerFrameBuffer.initialize(mDevice);

        return true;
    }

    inline void BoxApp::onResize()
    {
        D3DApplication::onResize();

        // The window resized, so update the aspect ratio and recompute the projection matrix.
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&mProjection, P);
    }

    inline void BoxApp::updateScene(const float dt)
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

    inline void BoxApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void BoxApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
    
    inline void BoxApp::buildVertexLayout(std::vector<char>& compiledShader)
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