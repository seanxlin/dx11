#pragma once

#include <cstdint>
#include <string>

#include <d3dx11.h>

#include "Timer.h"

namespace Framework
{
    class D3DApplication
    {
    public:
        D3DApplication(HINSTANCE hInstance);
        virtual ~D3DApplication();

        __forceinline HINSTANCE appInstance() const
        {
            return mAppInstance;
        }

        __forceinline HWND mainWindow() const
        {
            return mMainWindow;
        }

        __forceinline float aspectRatio() const
        {
            return static_cast<float>(mClientWidth) / mClientHeight;
        }

        int run();

        // Framework methods.  Derived client class overrides these methods to 
        // implement specific application requirements.
        virtual bool init();
        virtual void onResize(); 
        virtual void updateScene(const float dt) = 0;
        virtual void drawScene() = 0; 
        virtual LRESULT msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        // Convenience overrides for handling mouse input.
        virtual void onMouseDown(WPARAM btnState, const int32_t x, const int32_t y) { }
        virtual void onMouseUp(WPARAM btnState, const int32_t x, const int32_t y) { }
        virtual void onMouseMove(WPARAM btnState, const int32_t x, const int32_t y) { }

    protected:
        bool initMainWindow();
        bool initDirect3D();

        void calculateFrameStats();

    protected:
        HINSTANCE mAppInstance;
        HWND mMainWindow;

        Utils::Timer mTimer;

        ID3D11Device* mDevice;
        ID3D11DeviceContext* mImmediateContext;
        IDXGISwapChain* mSwapChain;
        ID3D11Texture2D* mDepthStencilBuffer;
        ID3D11RenderTargetView* mRenderTargetView;
        ID3D11DepthStencilView* mDepthStencilView;
        D3D11_VIEWPORT mScreenViewport;
        D3D_DRIVER_TYPE mDriverType;

        // Derived class should set these in derived constructor to customize starting values.
        std::wstring mMainWindowCaption;
        uint32_t mClientWidth;
        uint32_t mClientHeight;
        uint32_t m4xMsaaQuality;
        bool mIsPaused;
        bool mIsMinimized;
        bool mIsMaximized;
        bool mIsResizing;
        bool mEnable4xMsaa;
    };
}