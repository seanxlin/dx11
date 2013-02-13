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
    class ShadowMappingApp : public D3DApplication
    {
    public:
        inline ShadowMappingApp(HINSTANCE hInstance);

        inline ~ShadowMappingApp();

        inline bool init();

        inline void onResize();

        void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:       
        void drawFloor();
        void drawCylinder();

        void setShapesGeneralSettings();

        void updateInstancedBuffer();

        Utils::Camera mCamera;

        Utils::DirectionalLight mDirectionalLight[3];

        Utils::Material mFloorMaterial;
        Utils::Material mShapesMaterial;

        Shaders::ConstantBuffer<Shaders::FloorVSPerObjectBuffer> mFloorVSPerObjectBuffer;

        Shaders::ConstantBuffer<Shaders::ShapesVSPerObjectBuffer> mShapesVSPerObjectBuffer;

        Shaders::ConstantBuffer<Shaders::CommonPSPerFrameBuffer> mCommonPSPerFrameBuffer;
        Shaders::ConstantBuffer<Shaders::CommonPSPerObjectBuffer> mCommonPSPerObjectBuffer;
        
        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mFloorWorld;
        Managers::GeometryBuffersManager::InstancedData mCylinderWorld[5];

        // Define textures transformations
        DirectX::XMFLOAT4X4 mCommonTexTransform;

        POINT mLastMousePos;

        float mRotationAmmount;
    };     

    inline ShadowMappingApp::ShadowMappingApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mRotationAmmount(0.0f)
    {
        mMainWindowCaption = L"Instancing Demo";

        mCamera.setPosition(0.0f, 2.0f, -15.0f);

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        //
        // Set matrices
        //
        DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mFloorWorld, translation);
        
        translation = DirectX::XMMatrixTranslation(0.0f, 25.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mCylinderWorld[0].mWorld, translation);
        
        translation = DirectX::XMMatrixTranslation(50.0f, 25.0f, 50.0f);
        DirectX::XMStoreFloat4x4(&mCylinderWorld[1].mWorld, translation);

        translation = DirectX::XMMatrixTranslation(-50.0f, 25.0f, -50.0f);
        DirectX::XMStoreFloat4x4(&mCylinderWorld[2].mWorld, translation);

        translation = DirectX::XMMatrixTranslation(50.0f, 25.0f, -50.0f);
        DirectX::XMStoreFloat4x4(&mCylinderWorld[3].mWorld, translation);

        translation = DirectX::XMMatrixTranslation(-50.0f, 25.0f, 50.0f);
        DirectX::XMStoreFloat4x4(&mCylinderWorld[4].mWorld, translation);

        DirectX::XMMATRIX texTransform = DirectX::XMMatrixScaling(5.0f, 5.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mCommonTexTransform, texTransform);

        //
        // Directional lights.
        //
        mDirectionalLight[0].mAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight[0].mDiffuse = DirectX::XMFLOAT4(0.7f, 0.7f, 0.6f, 1.0f);
        mDirectionalLight[0].mSpecular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
        mDirectionalLight[0].mDirection = DirectX::XMFLOAT3(0.707f, 0.0f, 0.707f);

        mDirectionalLight[1].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        mDirectionalLight[1].mDiffuse = DirectX::XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
        mDirectionalLight[1].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight[1].mDirection = DirectX::XMFLOAT3(0.0f, -0.707f, 0.707f);

        mDirectionalLight[2].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        mDirectionalLight[2].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight[2].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight[2].mDirection = DirectX::XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);

        //
        // Initialize materials
        //
        mShapesMaterial.mAmbient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mShapesMaterial.mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mShapesMaterial.mSpecular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
        mShapesMaterial.mReflect = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

        mFloorMaterial.mAmbient  = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
        mFloorMaterial.mDiffuse  = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mFloorMaterial.mSpecular = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
        mFloorMaterial.mReflect  = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    inline ShadowMappingApp::~ShadowMappingApp()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();
    }

    inline bool ShadowMappingApp::init()
    {
        if(!D3DApplication::init())
            return false;

        mFloorVSPerObjectBuffer.initialize(mDevice);
        
        mShapesVSPerObjectBuffer.initialize(mDevice);

        mCommonPSPerFrameBuffer.initialize(mDevice);
        mCommonPSPerObjectBuffer.initialize(mDevice);
        
        Managers::ShadersManager::initAll(mDevice);   
        Managers::ResourcesManager::initAll(mDevice, mImmediateContext);
        Managers::PipelineStatesManager::initAll(mDevice);
        Managers::GeometryBuffersManager::initAll(mDevice);

        return true;
    }

    inline void ShadowMappingApp::onResize()
    {
        D3DApplication::onResize();

        mCamera.setLens(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
    }

    inline void ShadowMappingApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void ShadowMappingApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
}