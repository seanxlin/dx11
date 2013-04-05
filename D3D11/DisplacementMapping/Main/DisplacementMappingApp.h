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
    class DisplacementMappingApp : public D3DApplication
    {
    public:
        inline DisplacementMappingApp(HINSTANCE hInstance);

        inline ~DisplacementMappingApp();

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
        void drawSphere();
        void drawBox();

        void setShapesGeneralSettings();

        Camera mCamera;

        DirectionalLight mDirectionalLight[3];

        Material mFloorMaterial;
        Material mShapesMaterial;
        
        ConstantBuffer<Shaders::ShapesVSPerObjectBuffer> mShapesVSPerObjectBuffer;
        
        ConstantBuffer<Shaders::ShapesHSPerFrameBuffer> mShapesHSPerFrameBuffer;
        
        ConstantBuffer<Shaders::ShapesDSPerFrameBuffer> mShapesDSPerFrameBuffer;

        ConstantBuffer<Shaders::ShapesPSPerFrameBuffer> mShapesPSPerFrameBuffer;
        ConstantBuffer<Shaders::ShapesPSPerObjectBuffer> mShapesPSPerObjectBuffer;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mFloorWorld;
        DirectX::XMFLOAT4X4 mCylinderWorld;
        DirectX::XMFLOAT4X4 mSphereWorld;
        DirectX::XMFLOAT4X4 mBoxWorld;

        // Define textures transformations
        DirectX::XMFLOAT4X4 mShapesTexTransform;

        POINT mLastMousePos;

        float mTesselationFactor;

        bool mWireframeMode;

        float mRotationAmmount;
    };     

    inline DisplacementMappingApp::DisplacementMappingApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mTesselationFactor(0.0001f)
        , mWireframeMode(false)
        , mRotationAmmount(0.0f)
    {
        mMainWindowCaption = L"Displacement Mapping Demo";

        mCamera.mPosition = DirectX::XMFLOAT3(0.0f, 2.0f, -15.0f);

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        // Shapes world matrices
        DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mFloorWorld, translation);
        
        translation = DirectX::XMMatrixTranslation(0.0f, 25.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mCylinderWorld, translation);

        translation = DirectX::XMMatrixTranslation(50.0f, 25.0f, -50.0f);
        DirectX::XMStoreFloat4x4(&mSphereWorld, translation);
        
        translation = DirectX::XMMatrixTranslation(-50.0f, 25.0f, -50.0f);
        DirectX::XMStoreFloat4x4(&mBoxWorld, translation);

        // Texture transformation matrices
        DirectX::XMMATRIX texTransform = DirectX::XMMatrixScaling(5.0f, 5.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&mShapesTexTransform, texTransform);

        // Directional lights.
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

        // Initialize materials
        mShapesMaterial.mAmbient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mShapesMaterial.mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mShapesMaterial.mSpecular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
        mShapesMaterial.mReflect = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

        mFloorMaterial.mAmbient  = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
        mFloorMaterial.mDiffuse  = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mFloorMaterial.mSpecular = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
        mFloorMaterial.mReflect  = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    inline DisplacementMappingApp::~DisplacementMappingApp()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();
    }

    inline bool DisplacementMappingApp::init()
    {
        if(!D3DApplication::init())
            return false;
        
        ConstantBufferUtils::initialize(*mDevice, mShapesVSPerObjectBuffer);

        ConstantBufferUtils::initialize(*mDevice, mShapesHSPerFrameBuffer);

        ConstantBufferUtils::initialize(*mDevice, mShapesDSPerFrameBuffer);
        
        ConstantBufferUtils::initialize(*mDevice, mShapesPSPerFrameBuffer);
        ConstantBufferUtils::initialize(*mDevice, mShapesPSPerObjectBuffer);
        
        Managers::ShadersManager::initAll(mDevice);   
        Managers::ResourcesManager::initAll(mDevice, mImmediateContext);
        Managers::PipelineStatesManager::initAll(mDevice);
        Managers::GeometryBuffersManager::initAll(mDevice);

        return true;
    }

    inline void DisplacementMappingApp::onResize()
    {
        D3DApplication::onResize();

        CameraUtils::setFrustrum(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f, mCamera);
    }

    inline void DisplacementMappingApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mMainWindow);
    }

    inline void DisplacementMappingApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
}