#pragma once

#include <DirectXMath.h>

#include "HLSL/Buffers.h"

#include <Camera.h>
#include <ConstantBuffer.h>
#include <D3DApplication.h>
#include <LightHelper.h>

namespace Framework
{
    class GPUAcceleratedInterlockingTilesAlgorithmNaive : public D3DApplication
    {
    public:
        GPUAcceleratedInterlockingTilesAlgorithmNaive(HINSTANCE hInstance);

        ~GPUAcceleratedInterlockingTilesAlgorithmNaive();

        bool init();

        inline void onResize();

        void updateScene(const float dt);

        void drawScene(); 

        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);

        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);

        void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);

    private:       
        void drawGrid();

        Utils::Camera mCamera;

        Utils::DirectionalLight mDirectionalLight[3];

        Utils::Material mMaterial;
        
        Shaders::ConstantBuffer<Shaders::GridVSPerFrameBuffer> mGridVSPerObjectBuffer;

        Shaders::ConstantBuffer<Shaders::GridHSPerFrameBuffer> mGridHSPerFrameBuffer;

        Shaders::ConstantBuffer<Shaders::GridDSPerFrameBuffer> mGridDSPerFrameBuffer;

        Shaders::ConstantBuffer<Shaders::GridPSPerFrameBuffer> mGridPSPerFrameBuffer;


        // Define transformations from local spaces to world space.
        DirectX::XMFLOAT4X4 mWorldMatrix;

        POINT mLastMousePosition;

        bool mWireframeMode;
    };     

    inline void GPUAcceleratedInterlockingTilesAlgorithmNaive::onResize()
    {
        D3DApplication::onResize();

        mCamera.setLens(0.25f * DirectX::XM_PI, aspectRatio(), 1.0f, 1000.0f);
    }

    inline void GPUAcceleratedInterlockingTilesAlgorithmNaive::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y)
    {
        mLastMousePosition.x = x;
        mLastMousePosition.y = y;

        SetCapture(mMainWindow);
    }

    inline void GPUAcceleratedInterlockingTilesAlgorithmNaive::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y)
    {
        ReleaseCapture();
    }
}