//////////////////////////////////////////////////////////////////////////
// Demonstrates static cube mapping
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
    class StaticCubeMappingApp : public D3DApplication
    {
    public:
        inline StaticCubeMappingApp(HINSTANCE hInstance);

        inline ~StaticCubeMappingApp();

        inline bool init();

        inline void onResize();

        void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:       
        void drawLand();
        void drawSphere();
        void drawSky();

        Camera mCamera;

        Utils::DirectionalLight mDirectionalLight;

        Utils::Material mSandMaterial;
        Utils::Material mSphereMaterial;

        Shaders::ConstantBuffer<Shaders::LandPerFrameBuffer> mLandPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::LandVSPerObjectBuffer> mLandVSPerObjectBuffer;
        Shaders::ConstantBuffer<Shaders::LandPSPerObjectBuffer> mLandPSPerObjectBuffer;
        Shaders::ConstantBuffer<Shaders::SphereVSPerObjectBuffer> mSphereVSPerObjectBuffer;
        Shaders::ConstantBuffer<Shaders::SpherePSPerObjectBuffer> mSpherePSPerObjectBuffer;
        Shaders::ConstantBuffer<Shaders::SpherePSPerFrameBuffer> mSpherePSPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::SkyPerFrameBuffer> mSkyPerFrameBuffer;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mLandWorld;
        DirectX::XMFLOAT4X4 mSphereWorld;

        // Define textures transformations
        DirectX::XMFLOAT4X4 mSandTexTransform;
        DirectX::XMFLOAT4X4 mSphereTexTransform;

        POINT mLastMousePos;

        float mRotationAmmount;
    };     

    inline StaticCubeMappingApp::StaticCubeMappingApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mRotationAmmount(0.0f)
    {
        mMainWindowCaption = L"Static Cube Mapping Demo";

        mCamera.mPosition = DirectX::XMFLOAT3(0.0f, 2.0f, -15.0f);

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mLandWorld, I);
        
        DirectX::XMMATRIX sphereTranslation = DirectX::XMMatrixTranslation(0.0f, 100.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mSphereWorld, sphereTranslation);
        
        // Texture transformation matrices
        DirectX::XMMATRIX grassTexScale = DirectX::XMMatrixScaling(5.0f, 5.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mSandTexTransform, grassTexScale);
        DirectX::XMStoreFloat4x4(&mSphereTexTransform, I);

        // Directional light.
        mDirectionalLight.mAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight.mDiffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mDirectionalLight.mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
        mDirectionalLight.mDirection = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

        // Initialize materials
        mSandMaterial.mAmbient = DirectX::XMFLOAT4(0.956f, 0.643f, 0.376f, 1.0f);
        mSandMaterial.mDiffuse = DirectX::XMFLOAT4(0.956f, 0.643f, 0.376f, 1.0f);
        mSandMaterial.mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

        mSphereMaterial.mAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mSphereMaterial.mDiffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mSphereMaterial.mSpecular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mSphereMaterial.mReflect = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    }

    inline StaticCubeMappingApp::~StaticCubeMappingApp()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();
    }

    inline bool StaticCubeMappingApp::init()
    {
        if(!D3DApplication::init())
            return false;

        mLandPerFrameBuffer.initialize(*mDevice);
        mLandVSPerObjectBuffer.initialize(*mDevice);
        mLandPSPerObjectBuffer.initialize(*mDevice);
        mSphereVSPerObjectBuffer.initialize(*mDevice);
        mSpherePSPerObjectBuffer.initialize(*mDevice);
        mSpherePSPerFrameBuffer.initialize(*mDevice);
        mSkyPerFrameBuffer.initialize(*mDevice);
        
        Managers::ShadersManager::initAll(mDevice);   
        Managers::ResourcesManager::initAll(mDevice, mImmediateContext);
        Managers::PipelineStatesManager::initAll(mDevice);
        Managers::GeometryBuffersManager::initAll(mDevice);

        return true;
    }

    inline void StaticCubeMappingApp::onResize()
    {
        D3DApplication::onResize();

        CameraUtils::setFrustrum(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f, mCamera);
    }

    inline void StaticCubeMappingApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void StaticCubeMappingApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
}