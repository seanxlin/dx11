//////////////////////////////////////////////////////////////////////////
// Demonstrates rendering some shapes.
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
#include <DxErrorChecker.h>

namespace Framework
{
    class ShapesApp : public D3DApplication
    {
    public:
        inline ShapesApp(HINSTANCE hInstance);

        inline ~ShapesApp();

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

        inline void buildRasterizerState();

    private:
        ID3D11Buffer* mVertexBuffer;
        ID3D11Buffer* mIndexBuffer;

        ID3D11VertexShader* mVertexShader;
        ID3D11PixelShader* mPixelShader;

        ConstantBuffer<Shaders::PerObjectBuffer> mPerObjectBuffer;

        ID3D11InputLayout* mInputLayout;
        
        ID3D11RasterizerState* mWireframeRS;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mSphereWorld[10];
        DirectX::XMFLOAT4X4 mCylWorld[10];
        DirectX::XMFLOAT4X4 mBoxWorld;
        DirectX::XMFLOAT4X4 mGridWorld;
        DirectX::XMFLOAT4X4 mCenterSphere;

        DirectX::XMFLOAT4X4 mView;
        DirectX::XMFLOAT4X4 mProjection;

        uint32_t mBoxVertexOffset;
        uint32_t mGridVertexOffset;
        uint32_t mSphereVertexOffset;
        uint32_t mCylinderVertexOffset;

        uint32_t mBoxIndexOffset;
        uint32_t mGridIndexOffset;
        uint32_t mSphereIndexOffset;
        uint32_t mCylinderIndexOffset;

        uint32_t mBoxIndexCount;
        uint32_t mGridIndexCount;
        uint32_t mSphereIndexCount;
        uint32_t mCylinderIndexCount;

        float mTheta;
        float mPhi;
        float mRadius;
        
        POINT mLastMousePos;
    };     

    inline ShapesApp::ShapesApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mVertexBuffer(nullptr)
        , mIndexBuffer(nullptr)
        , mVertexShader(nullptr)
        , mPixelShader(nullptr)
        , mInputLayout(nullptr)
        , mWireframeRS(nullptr)
        , mBoxVertexOffset(0)
        , mGridVertexOffset(0)
        , mSphereVertexOffset(0)
        , mCylinderVertexOffset(0)
        , mBoxIndexOffset(0)
        , mGridIndexOffset(0)            
        , mSphereIndexOffset(0)
        , mCylinderIndexOffset(0)
        , mBoxIndexCount(0)
        , mGridIndexCount(0)
        , mSphereIndexCount(0)
        , mCylinderIndexCount(0)
        , mTheta(1.5f * DirectX::XM_PI)
        , mPhi(0.25f * DirectX::XM_PI)
        , mRadius(5.0f)
    {
        mMainWindowCaption = L"Shapes Demo";

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mGridWorld, I);
        DirectX::XMStoreFloat4x4(&mView, I);
        DirectX::XMStoreFloat4x4(&mProjection, I);

        DirectX::XMMATRIX boxScale = DirectX::XMMatrixScaling(2.0f, 1.0f, 2.0f);
        DirectX::XMMATRIX boxOffset = DirectX::XMMatrixTranslation(0.0f, 0.5f, 0.0f);
        XMStoreFloat4x4(&mBoxWorld, DirectX::XMMatrixMultiply(boxScale, boxOffset));

        DirectX::XMMATRIX centerSphereScale = DirectX::XMMatrixScaling(2.0f, 2.0f, 2.0f);
        DirectX::XMMATRIX centerSphereOffset = DirectX::XMMatrixTranslation(0.0f, 2.0f, 0.0f);
        XMStoreFloat4x4(&mCenterSphere, DirectX::XMMatrixMultiply(centerSphereScale, centerSphereOffset));

        for(size_t i = 0; i < 5; ++i)
        {
            DirectX::XMStoreFloat4x4(&mCylWorld[i * 2 + 0], DirectX::XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
            DirectX::XMStoreFloat4x4(&mCylWorld[i * 2 + 1], DirectX::XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

            DirectX::XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], DirectX::XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
            DirectX::XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], DirectX::XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
        }
    }

    inline ShapesApp::~ShapesApp()
    {
        mVertexBuffer->Release();
        mIndexBuffer->Release();
        mVertexShader->Release();
        mPixelShader->Release();
        mInputLayout->Release();
        mWireframeRS->Release();
    }

    inline bool ShapesApp::init()
    {
        if(!D3DApplication::init())
            return false;

        buildGeometryBuffers();
        buildShaders();            
        buildRasterizerState();
        ConstantBufferUtils::initialize(*mDevice, mPerObjectBuffer);

        return true;
    }

    inline void ShapesApp::onResize()
    {
        D3DApplication::onResize();

        // The window resized, so update the aspect ratio and recompute the projection matrix.
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&mProjection, P);
    }

    inline void ShapesApp::updateScene(const float dt)
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

    inline void ShapesApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void ShapesApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }

    inline void ShapesApp::buildVertexLayout(std::vector<char>& compiledShader)
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

        DxErrorChecker(result);
    }

    inline void ShapesApp::buildRasterizerState()
    {
        D3D11_RASTERIZER_DESC wireframeDesc;
        ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
        wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
        wireframeDesc.CullMode = D3D11_CULL_BACK;
        wireframeDesc.FrontCounterClockwise = false;
        wireframeDesc.DepthClipEnable = true;

        HRESULT result = mDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS);
        DxErrorChecker(result);
    }
}