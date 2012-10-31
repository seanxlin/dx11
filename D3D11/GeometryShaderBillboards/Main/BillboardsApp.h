//////////////////////////////////////////////////////////////////////////
// Demonstrates texturing an scene.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>

#include "HLSL/Buffers.h"
#include "Managers/GeometryBuffersManager.h"
#include "Managers/ResourcesManager.h"
#include "Managers/PipelineStatesManager.h"
#include "Managers/ShadersManager.h"

#include <ConstantBuffer.h>
#include <D3DApplication.h>
#include <LightHelper.h>

namespace Framework
{
    class BillboardsApp : public D3DApplication
    {
    public:
        inline BillboardsApp(HINSTANCE hInstance);

        inline ~BillboardsApp();

        inline bool init();

        inline void onResize();

        void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:       
        void drawBillboards(DirectX::CXMMATRIX viewProjection);
        void drawLand(DirectX::CXMMATRIX viewProjection);

        Utils::DirectionalLight mDirectionalLight;
        Utils::SpotLight mSpotLight;

        Utils::Material mSandMaterial;
        Utils::Material mPalmMaterial;

        Shaders::ConstantBuffer<Shaders::CommonPerFrameBuffer> mCommonPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::LandPerObjectBuffer> mLandPerObjectBuffer;
        Shaders::ConstantBuffer<Shaders::BillboardsPerObjectBuffer> mBillboardsPerObjectBuffer;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mLandWorld;

        // Define textures transformations
        DirectX::XMFLOAT4X4 mSandTexTransform;

        DirectX::XMFLOAT4X4 mView;
        DirectX::XMFLOAT4X4 mProjection;

        DirectX::XMFLOAT3 mEyePositionW;

        float mTheta;
        float mPhi;
        float mRadius;

        POINT mLastMousePos;

        bool mAlphaToCoverageOn;
    };     

    inline BillboardsApp::BillboardsApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mEyePositionW(0.0f, 0.0f, 0.0f)
        , mTheta(1.5f * DirectX::XM_PI)
        , mPhi(0.25f * DirectX::XM_PI)
        , mRadius(5.0f)
        , mAlphaToCoverageOn(true)
    {
        mMainWindowCaption = L"Billboard Demo";
        mEnable4xMsaa = true;

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mLandWorld, I);
        DirectX::XMStoreFloat4x4(&mView, I);
        DirectX::XMStoreFloat4x4(&mProjection, I);  
        
        // Texture transformation matrices
        DirectX::XMMATRIX grassTexScale = DirectX::XMMatrixScaling(5.0f, 5.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mSandTexTransform, grassTexScale);

        // Directional light.
        mDirectionalLight.mAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight.mDiffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mDirectionalLight.mSpecular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mDirectionalLight.mDirection = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

        // Spot light--position and direction changed every frame to animate in UpdateScene function.
        mSpotLight.mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        mSpotLight.mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
        mSpotLight.mSpecular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mSpotLight.mAttenuation = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
        mSpotLight.mSpot = 96.0f;
        mSpotLight.mRange = 10000.0f;

        // Initialize materials
        mSandMaterial.mAmbient = DirectX::XMFLOAT4(0.956f, 0.643f, 0.376f, 1.0f);
        mSandMaterial.mDiffuse = DirectX::XMFLOAT4(0.956f, 0.643f, 0.376f, 1.0f);
        mSandMaterial.mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

        mPalmMaterial.mAmbient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mPalmMaterial.mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mPalmMaterial.mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
    }

    inline BillboardsApp::~BillboardsApp()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();
    }

    inline bool BillboardsApp::init()
    {
        if(!D3DApplication::init())
            return false;
        
        mCommonPerFrameBuffer.initialize(mDevice);
        mLandPerObjectBuffer.initialize(mDevice);
        mBillboardsPerObjectBuffer.initialize(mDevice);
        
        Managers::ShadersManager::initAll(mDevice);   
        Managers::ResourcesManager::initAll(mDevice, mImmediateContext);
        Managers::PipelineStatesManager::initAll(mDevice);
        Managers::GeometryBuffersManager::initAll(mDevice);

        return true;
    }

    inline void BillboardsApp::onResize()
    {
        D3DApplication::onResize();

        // The window resized, so update the aspect ratio and recompute the projection matrix.
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&mProjection, P);
    }

    inline void BillboardsApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void BillboardsApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
}