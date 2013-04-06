#pragma once

#include <DirectXMath.h>

#include <Camera.h>
#include <ConstantBuffer.h>
#include <DxErrorChecker.h>
#include <LightHelper.h>

#include "HLSL/Buffers.h"

#include "Direct3D.h"

    class GPUAcceleratedInterlockingTilesAlgorithmNaive
    {
    public:
        GPUAcceleratedInterlockingTilesAlgorithmNaive();

        ~GPUAcceleratedInterlockingTilesAlgorithmNaive();

        bool init(Direct3DData& direct3DData, WindowData& windowData);

        inline void onResize(Direct3DData& direct3DData, WindowData& windowData);

        void updateScene(const float dt);

        void drawScene(Direct3DData& direct3DData); 

        inline void onMouseDown(WPARAM btnState, 
                                const int32_t x, 
                                const int32_t y,
                                WindowData& windowData);

        inline void onMouseUp(WPARAM btnState, 
                              const int32_t x, 
                              const int32_t y);

        void onMouseMove(WPARAM btnState, 
                         const int32_t x, 
                         const int32_t y);

        int run(Direct3DData& direct3DData, 
                WindowState& windowState,
                WindowData& windowData);

        LRESULT msgProc(HWND hwnd, 
                        UINT msg, 
                        WPARAM wParam, 
                        LPARAM lParam);

    private:       
        void drawGrid(Direct3DData& direct3DData);
        bool initMainWindow(WindowData& windowData);
        bool initDirect3D(Direct3DData& direct3DData, WindowData& windowData);

        void calculateFrameStats(WindowData& windowData);
        
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

    inline void GPUAcceleratedInterlockingTilesAlgorithmNaive::onResize(Direct3DData& direct3DData, WindowData& windowData)
    {
        assert(direct3DData.mImmediateContext);
        assert(direct3DData.mDevice);
        assert(direct3DData.mSwapChain);

        // Release the old views, as they hold references to the buffers we
        // will be destroying. Also release the old depth/stencil buffer.
        if (direct3DData.mRenderTargetView)
        {
            direct3DData.mRenderTargetView->Release();
        }

        if (direct3DData.mDepthStencilView)
        {
            direct3DData.mDepthStencilView->Release();
        }

        if (direct3DData.mDepthStencilBuffer)
        {
            direct3DData.mDepthStencilBuffer->Release();
        }

        // Resize the swap chain and recreate the render target view.
        HRESULT result = direct3DData.mSwapChain->ResizeBuffers(
            1, 
            windowData.mClientWidth, 
            windowData.mClientHeight, 
            DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        
        DxErrorChecker(result);

        ID3D11Texture2D* backBuffer = nullptr;
        result = direct3DData.mSwapChain->GetBuffer(
            0, 
            __uuidof(ID3D11Texture2D), 
            reinterpret_cast<void**>(&backBuffer));
        
        DxErrorChecker(result);

        result = direct3DData.mDevice->CreateRenderTargetView(
            backBuffer, 
            0, 
            &direct3DData.mRenderTargetView);
        
        DxErrorChecker(result);
        backBuffer->Release();

        // Create the depth/stencil buffer and view.
        D3D11_TEXTURE2D_DESC depthStencilDesc;	
        depthStencilDesc.Width = windowData.mClientWidth;
        depthStencilDesc.Height = windowData.mClientHeight;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        // Use 4X MSAA? --must match swap chain MSAA values.
        if (direct3DData.mEnable4xMsaa)
        {
            depthStencilDesc.SampleDesc.Count = 4;
            depthStencilDesc.SampleDesc.Quality = direct3DData.m4xMsaaQuality - 1;
        }
        
        // No MSAA
        else
        {
            depthStencilDesc.SampleDesc.Count = 1;
            depthStencilDesc.SampleDesc.Quality = 0;
        }

        depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesc.CPUAccessFlags = 0; 
        depthStencilDesc.MiscFlags = 0;

        result = direct3DData.mDevice->CreateTexture2D(
            &depthStencilDesc, 
            0, 
            &direct3DData.mDepthStencilBuffer);
        
        DxErrorChecker(result);

        result = direct3DData.mDevice->CreateDepthStencilView(
            direct3DData.mDepthStencilBuffer,
            0, 
            &direct3DData.mDepthStencilView);
        
        DxErrorChecker(result);

        // Bind the render target view and depth/stencil view to the pipeline.
        direct3DData.mImmediateContext->OMSetRenderTargets(
            1, 
            &direct3DData.mRenderTargetView, 
            direct3DData.mDepthStencilView);

        // Set the viewport transform.
        direct3DData.mScreenViewport.TopLeftX = 0;
        direct3DData.mScreenViewport.TopLeftY = 0;
        direct3DData.mScreenViewport.Width = static_cast<float>(windowData.mClientWidth);
        direct3DData.mScreenViewport.Height = static_cast<float>(windowData.mClientHeight);
        direct3DData.mScreenViewport.MinDepth = 0.0f;
        direct3DData.mScreenViewport.MaxDepth = 1.0f;

        direct3DData.mImmediateContext->RSSetViewports(1, &direct3DData.mScreenViewport);

        const float aspectRatio = static_cast<float> (windowData.mClientWidth / windowData.mClientHeight);
        CameraUtils::setFrustrum(
            0.25f * DirectX::XM_PI, 
            aspectRatio, 
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

    inline void GPUAcceleratedInterlockingTilesAlgorithmNaive::onMouseUp(WPARAM btnState, 
                                                                         const int32_t x, 
                                                                         const int32_t y)
    {
        ReleaseCapture();
    }