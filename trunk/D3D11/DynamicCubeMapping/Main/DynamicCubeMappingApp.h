//////////////////////////////////////////////////////////////////////////
// Demonstrates dynamic cube mapping
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
    class DynamicCubeMappingApp : public D3DApplication
    {
    public:
        inline DynamicCubeMappingApp(HINSTANCE hInstance);

        inline ~DynamicCubeMappingApp();

        inline bool init();

        inline void onResize();

        void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:       
        void buildCubeFaceCamera(const float x, const float y, const float z);

        void drawScene(Utils::Camera& camera, const bool isSphereDrawable);
        void drawLand(Utils::Camera& camera);
        void drawSphere(Utils::Camera& camera);
        void drawSky(Utils::Camera& camera);

        Utils::Camera mCamera;
        Utils::Camera mCubeMapCamera[6];

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

        D3D11_VIEWPORT mCubeMapViewport;

        POINT mLastMousePos;

        float mRotationAmmount;
    };     

    inline DynamicCubeMappingApp::DynamicCubeMappingApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mRotationAmmount(0.0f)
    {
        mMainWindowCaption = L"Dynamic Cube Mapping Demo";

        mCamera.setPosition(0.0f, 2.0f, -15.0f);

        buildCubeFaceCamera(0.0f, 2.0f, 0.0f);

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

    inline DynamicCubeMappingApp::~DynamicCubeMappingApp()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();
    }

    inline bool DynamicCubeMappingApp::init()
    {
        if(!D3DApplication::init())
            return false;

        mLandPerFrameBuffer.initialize(mDevice);
        mLandVSPerObjectBuffer.initialize(mDevice);
        mLandPSPerObjectBuffer.initialize(mDevice);
        mSphereVSPerObjectBuffer.initialize(mDevice);
        mSpherePSPerObjectBuffer.initialize(mDevice);
        mSpherePSPerFrameBuffer.initialize(mDevice);
        mSkyPerFrameBuffer.initialize(mDevice);
        
        Managers::ShadersManager::initAll(mDevice);   
        Managers::ResourcesManager::initAll(mDevice, mImmediateContext);
        Managers::PipelineStatesManager::initAll(mDevice);
        Managers::GeometryBuffersManager::initAll(mDevice);
        
        // Viewport for drawing into cubemap.
        mCubeMapViewport.TopLeftX = 0.0f;
        mCubeMapViewport.TopLeftY = 0.0f;
        mCubeMapViewport.Width = 256.0f;
        mCubeMapViewport.Height = 256.0f;
        mCubeMapViewport.MinDepth = 0.0f;
        mCubeMapViewport.MaxDepth = 1.0f;

        return true;
    }

    inline void DynamicCubeMappingApp::onResize()
    {
        D3DApplication::onResize();

        mCamera.setLens(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
    }

    inline void DynamicCubeMappingApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void DynamicCubeMappingApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
}