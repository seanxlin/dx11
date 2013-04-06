#pragma once

#include <DirectXMath.h>

#include "HLSL/Buffers.h"

#include <Camera.h>
#include <ConstantBuffer.h>
#include <LightHelper.h>

#include "Direct3D.h"

    class GPUAcceleratedInterlockingTilesAlgorithmNaive : public D3DApplication
    {
    public:
        GPUAcceleratedInterlockingTilesAlgorithmNaive();

        ~GPUAcceleratedInterlockingTilesAlgorithmNaive();

        bool init(Direct3DData& direct3DData, WindowData& windowData);

        inline void onResize(Direct3DData& direct3DData);

        void updateScene(const float dt);

        void drawScene(Direct3DData& direct3DData); 

        inline void onMouseDown(WPARAM btnState, 
                                const int32_t x, 
                                const int32_t y,
                                WindowData& windowData);

        inline void onMouseUp(WPARAM btnState, 
                              const int32_t x, 
                              const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:       
        void drawGrid(Direct3DData& direct3DData);

        Camera mCamera;

        DirectionalLight mDirectionalLight[3];

        Material mTerrainMaterial;
        
        ConstantBuffer<GridVSPerFrameBuffer> mGridVSPerObjectBuffer;

        ConstantBuffer<GridHSPerFrameBuffer> mGridHSPerFrameBuffer;

        ConstantBuffer<GridDSPerFrameBuffer> mGridDSPerFrameBuffer;

        ConstantBuffer<GridPSPerFrameBuffer> mGridPSPerFrameBuffer;
        
        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mWorldMatrix;
        DirectX::XMFLOAT4X4 mTextureScaleMatrix;

        POINT mLastMousePosition;

        bool mWireframeMode;
    };     

    inline void GPUAcceleratedInterlockingTilesAlgorithmNaive::onResize(Direct3DData& direct3DData)
    {
        D3DApplication::onResize(direct3DData);

        CameraUtils::setFrustrum(0.25f * DirectX::XM_PI, 
                                 aspectRatio(), 
                                 1.0f, 
                                 1000.0f, 
                                 mCamera);
    }

    inline void GPUAcceleratedInterlockingTilesAlgorithmNaive::onMouseDown(WPARAM btnState, 
                                                                           const int32_t x, 
                                                                           const int32_t y,
                                                                           WindowData& windowData)
    {
        mLastMousePosition.x = x;
        mLastMousePosition.y = y;

        SetCapture(windowData.mMainWindow);
    }

    inline void GPUAcceleratedInterlockingTilesAlgorithmNaive::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }