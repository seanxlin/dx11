#pragma once

#include <cassert>
#include <DirectXColors.h>

#include <D3DApplication.h>
#include <DxErrorChecker.h>

namespace Framework
{
    class InitD3DApp : public Framework::D3DApplication
    {
    public:
        inline InitD3DApp(HINSTANCE hInstance);

        inline InitD3DApp::~InitD3DApp();

        inline bool InitD3DApp::init();

        inline void InitD3DApp::onResize();

        inline void InitD3DApp::updateScene(const float dt);

        inline void InitD3DApp::drawScene();
        
        inline void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y);
        inline void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y);
        inline void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y);
    };

    inline InitD3DApp::InitD3DApp(HINSTANCE hInstance)
        : Framework::D3DApplication(hInstance) 
    {
    }

    inline InitD3DApp::~InitD3DApp()
    {
    }

    inline bool InitD3DApp::init()
    {
        return D3DApplication::init();
    }

    inline void InitD3DApp::onResize()
    {
        D3DApplication::onResize();
    }

    inline void InitD3DApp::updateScene(const float dt)
    {

    }

    inline void InitD3DApp::drawScene()
    {
        assert(mImmediateContext);
        assert(mSwapChain);
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Blue));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        HRESULT result = mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    inline void InitD3DApp::onMouseDown(WPARAM btnState, const int32_t x, const int32_t y) {}
    inline void InitD3DApp::onMouseUp(WPARAM btnState, const int32_t x, const int32_t y) {}
    inline void InitD3DApp::onMouseMove(WPARAM btnState, const int32_t x, const int32_t y) {}
}