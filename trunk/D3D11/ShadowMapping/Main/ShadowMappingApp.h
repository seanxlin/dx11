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
#include <ShadowMapper.h>

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
        void drawSceneToShadowMap();
        void drawFloor();
        void drawCylinders();

        void drawCylindersIntoShadowMap();
        void drawFloorIntoShadowMap();

        void setShapesGeneralSettings();

        void buildShadowTransform();

        Camera mCamera;

        DirectionalLight mDirectionalLight[3];

        Material mFloorMaterial;
        Material mShapesMaterial;

        ConstantBuffer<Shaders::FloorVSPerObjectBuffer> mFloorVSPerObjectBuffer;

        ConstantBuffer<Shaders::ShapesVSPerObjectBuffer> mShapesVSPerObjectBuffer;

        ConstantBuffer<Shaders::CommonPSPerFrameBuffer> mCommonPSPerFrameBuffer;
        ConstantBuffer<Shaders::CommonPSPerObjectBuffer> mCommonPSPerObjectBuffer;
        
        ConstantBuffer<Shaders::ShadowMapVSPerObjectBuffer> mShadowMapVSPerObjectBuffer;
        ConstantBuffer<Shaders::FloorShadowMapVSPerObjectBuffer> mFloorShadowMapVSPerObjectBuffer;

        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mFloorWorld;

        // Define textures transformations
        DirectX::XMFLOAT4X4 mCommonTexTransform;

        POINT mLastMousePos;

        Utils::BoundingSphere mSceneBounds;
        static const int sShadowMapSize = 2048;
        Utils::ShadowMapper* mShadowMapper;
        DirectX::XMFLOAT4X4 mLightView;
        DirectX::XMFLOAT4X4 mLightProjection;
        DirectX::XMFLOAT4X4 mShadowTransform;

        DirectX::XMFLOAT3 mDefaultShadowLightDirection[3];
        float mLightRotationAngle;
    };     

    inline ShadowMappingApp::ShadowMappingApp(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mShadowMapper(nullptr)
        , mLightRotationAngle(0.0f)
    {
        mMainWindowCaption = L"Shadow Mapping Demo";

        mCamera.mPosition = DirectX::XMFLOAT3(0.0f, 2.0f, -15.0f);

        mLastMousePos.x = 0;
        mLastMousePos.y = 0;

        //
        // Set matrices
        //
        DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mFloorWorld, translation);

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

        mDefaultShadowLightDirection[0] = mDirectionalLight[0].mDirection;
        mDefaultShadowLightDirection[1] = mDirectionalLight[1].mDirection;
        mDefaultShadowLightDirection[2] = mDirectionalLight[2].mDirection;

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

        // Estimate the scene bounding sphere manually since we know how the scene was constructed.
        // The grid is the "widest object" with a width of 20 and depth of 30.0f, and centered at
        // the world space origin.  In general, you need to loop over every world space vertex
        // position and compute the bounding sphere.
        mSceneBounds.mCenter = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        mSceneBounds.mRadius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);
    }

    inline ShadowMappingApp::~ShadowMappingApp()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();

        delete mShadowMapper;
    }

    inline bool ShadowMappingApp::init()
    {
        if(!D3DApplication::init())
            return false;

        ConstantBufferUtils::initialize(*mDevice, mFloorVSPerObjectBuffer);
        
        ConstantBufferUtils::initialize(*mDevice, mShapesVSPerObjectBuffer);

        ConstantBufferUtils::initialize(*mDevice, mCommonPSPerFrameBuffer);
        ConstantBufferUtils::initialize(*mDevice, mCommonPSPerObjectBuffer);
        
        ConstantBufferUtils::initialize(*mDevice, mShadowMapVSPerObjectBuffer);
        ConstantBufferUtils::initialize(*mDevice, mFloorShadowMapVSPerObjectBuffer);

        Managers::ShadersManager::initAll(mDevice);   
        Managers::ResourcesManager::initAll(mDevice, mImmediateContext);
        Managers::PipelineStatesManager::initAll(mDevice);
        Managers::GeometryBuffersManager::initAll(mDevice);

        mShadowMapper = new Utils::ShadowMapper(*mDevice, sShadowMapSize, sShadowMapSize);

        return true;
    }

    inline void ShadowMappingApp::onResize()
    {
        D3DApplication::onResize();

        CameraUtils::setFrustrum(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f, mCamera);
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