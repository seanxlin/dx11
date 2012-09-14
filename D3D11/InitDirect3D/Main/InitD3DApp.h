#pragma once

#include <cassert>
#include <DirectXColors.h>

#include "D3DApplication.h"
#include "D3DErrorChecker.h"

namespace Framework
{
    class InitD3DApp : public Framework::D3DApplication
    {
    public:
        InitD3DApp::InitD3DApp(HINSTANCE hInstance)
            : Framework::D3DApplication(hInstance) 
        {
        }

        InitD3DApp::~InitD3DApp()
        {
        }

        bool InitD3DApp::init()
        {
            return D3DApplication::init();
        }

        void InitD3DApp::onResize()
        {
            D3DApplication::onResize();
        }

        void InitD3DApp::updateScene(const float dt)
        {

        }

        void InitD3DApp::drawScene()
        {
            assert(mImmediateContext);
            assert(mSwapChain);
            mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Blue));
            mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            HRESULT result = mSwapChain->Present(0, 0);
            DebugUtils::ErrorChecker(result);
        }

    };
}