//////////////////////////////////////////////////////////////////////////
// Demonstrates blur effect in Compute Shader
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

#include "BlurFilter.h"

namespace Framework
{
    class BlurApp : public D3DApplication
    {
    public:
        inline BlurApp(HINSTANCE hInstance);

        inline ~BlurApp();

        inline bool init();

        inline void onResize();

        void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:       
        void drawLand();
        void drawScreenQuad();

        void buildOffscreenViews();

        Camera mCamera;

        Framework::BlurFilter mBlurFilter;

        Utils::DirectionalLight mDirectionalLight;
        Utils::SpotLight mSpotLight;

        Utils::Material mSandMaterial;

        Shaders::ConstantBuffer<Shaders::LandPerFrameBuffer> mLandPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::LandPerObjectBuffer> mLandPerObjectBuffer;
        Shaders::ConstantBuffer<Shaders::ScreenQuadVSPerFrameBuffer> mScreenQuadVSPerFrameBuffer;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mLandWorld;

        // Define textures transformations
        DirectX::XMFLOAT4X4 mSandTexTransform;

        POINT mLastMousePos;
    };     

    inline BlurApp::BlurApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
    {
        mMainWindowCaption = L"Blur Demo";

        mCamera.mPosition = DirectX::XMFLOAT3(0.0f, 2.0f, -15.0f);

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mLandWorld, I);
        
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
    }

    inline BlurApp::~BlurApp()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();
    }

    inline bool BlurApp::init()
    {
        if(!D3DApplication::init())
            return false;

        mBlurFilter.init(mDevice, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
        
        mLandPerFrameBuffer.initialize(*mDevice);
        mLandPerObjectBuffer.initialize(*mDevice);
        mScreenQuadVSPerFrameBuffer.initialize(*mDevice);
        
        Managers::ShadersManager::initAll(mDevice);   
        Managers::ResourcesManager::initAll(mDevice, mImmediateContext);
        Managers::PipelineStatesManager::initAll(mDevice);
        Managers::GeometryBuffersManager::initAll(mDevice);

        buildOffscreenViews();

        return true;
    }

    inline void BlurApp::onResize()
    {
        D3DApplication::onResize();

        buildOffscreenViews();
        mBlurFilter.init(mDevice, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

        CameraUtils::setFrustrum(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f, mCamera);
    }

    inline void BlurApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void BlurApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
}