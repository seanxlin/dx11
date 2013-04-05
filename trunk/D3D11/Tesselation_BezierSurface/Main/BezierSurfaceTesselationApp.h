//////////////////////////////////////////////////////////////////////////
// Demonstrates bezier surfaces using tessellation
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>

#include "HLSL/Buffers.h"
#include "Managers/GeometryBuffersManager.h"
#include "Managers/ResourcesManager.h"
#include "Managers/PipelineStatesManager.h"
#include "Managers/ShadersManager.h"

#include <Camera.h>
#include <ConstantBuffer.h>
#include <D3DApplication.h>
#include <LightHelper.h>

namespace Framework
{
    class BezierSurfaceTesselationApp : public D3DApplication
    {
    public:
        inline BezierSurfaceTesselationApp(HINSTANCE hInstance);

        inline ~BezierSurfaceTesselationApp();

        inline bool init();

        inline void onResize();

        void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:       
        void drawBezierSurface();

        Camera mCamera;

        Utils::DirectionalLight mDirectionalLight;

        Utils::Material mSandMaterial;
        Shaders::ConstantBuffer<Shaders::BezierSurfaceDSPerFrameBuffer> mBezierSurfaceDSPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::BezierSurfaceHSPerFrameBuffer> mBezierSurfaceHSPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::BezierSurfacePSPerFrameBuffer> mBezierSurfacePSPerFrameBuffer;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mBezierSurfaceWorld;

        // Define textures transformations
        DirectX::XMFLOAT4X4 mSandTexTransform;

        POINT mLastMousePos;

        float mTesselationFactor;

        bool mWireframeMode;

        float mRotationAmmount;
    };     

    inline BezierSurfaceTesselationApp::BezierSurfaceTesselationApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mRotationAmmount(0.0f)
        , mTesselationFactor(64.0f)
        , mWireframeMode(false)
    {
        mMainWindowCaption = L"Tesselation - Bezier Surface";

        mCamera.mPosition = DirectX::XMFLOAT3(0.0f, 2.0f, -15.0f);

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mBezierSurfaceWorld, I);
        
        // Texture transformation matrices
        DirectX::XMMATRIX textureScale = DirectX::XMMatrixScaling(5.0f, 5.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mSandTexTransform, textureScale);

        // Directional light.
        mDirectionalLight.mAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight.mDiffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mDirectionalLight.mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
        mDirectionalLight.mDirection = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

        // Initialize materials
        mSandMaterial.mAmbient = DirectX::XMFLOAT4(0.956f, 0.643f, 0.376f, 1.0f);
        mSandMaterial.mDiffuse = DirectX::XMFLOAT4(0.956f, 0.643f, 0.376f, 1.0f);
        mSandMaterial.mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
    }

    inline BezierSurfaceTesselationApp::~BezierSurfaceTesselationApp()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();
    }

    inline bool BezierSurfaceTesselationApp::init()
    {
        if(!D3DApplication::init())
            return false;

        mBezierSurfaceDSPerFrameBuffer.initialize(*mDevice);
        mBezierSurfaceHSPerFrameBuffer.initialize(*mDevice);
        mBezierSurfacePSPerFrameBuffer.initialize(*mDevice);
        
        Managers::ShadersManager::initAll(mDevice);   
        Managers::ResourcesManager::initAll(mDevice, mImmediateContext);
        Managers::PipelineStatesManager::initAll(mDevice);
        Managers::GeometryBuffersManager::initAll(mDevice);

        return true;
    }

    inline void BezierSurfaceTesselationApp::onResize()
    {
        D3DApplication::onResize();

        CameraUtils::setFrustrum(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f, mCamera);
    }

    inline void BezierSurfaceTesselationApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void BezierSurfaceTesselationApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
}