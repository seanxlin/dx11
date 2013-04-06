#pragma once

#include <cstdint>
#include <d3d11.h>
#include <string>

#include <Timer.h>

struct WindowState
{
    WindowState()
        : mIsPaused(false)
        , mIsMinimized(false)
        , mIsMaximized(false)
        , mIsResizing(false)
    {

    }

    bool mIsPaused;
    bool mIsMinimized;
    bool mIsMaximized;
    bool mIsResizing;
};

static WindowState gWindowState;

struct WindowData
{
    WindowData()
    {
        ZeroMemory(&mAppInstance, sizeof(HINSTANCE));
        ZeroMemory(&mMainWindow, sizeof(HWND));
    }

    HINSTANCE mAppInstance;
    HWND mMainWindow;
};

static WindowData gWindowData;

struct Direct3DData
{
    Direct3DData()
        : mDevice(nullptr)
        , mImmediateContext(nullptr)
        , mSwapChain(nullptr)
        , mDepthStencilBuffer(nullptr)
        , mRenderTargetView(nullptr)
        , mDepthStencilView(nullptr)
        , mDriverType(D3D_DRIVER_TYPE_HARDWARE)
    {
        ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));
    }

    ~Direct3DData()
    {
        /*mRenderTargetView->Release();
        mDepthStencilView->Release();
        mSwapChain->Release();
        mDepthStencilBuffer->Release();

        // Restore all default settings.
        mImmediateContext->ClearState();

        mImmediateContext->Release();
        mDevice->Release();*/
    }

    ID3D11Device* mDevice;
    ID3D11DeviceContext* mImmediateContext;
    IDXGISwapChain* mSwapChain;
    ID3D11Texture2D* mDepthStencilBuffer;
    ID3D11RenderTargetView* mRenderTargetView;
    ID3D11DepthStencilView* mDepthStencilView;
    D3D11_VIEWPORT mScreenViewport;
    D3D_DRIVER_TYPE mDriverType;
};

static Direct3DData gDirect3DData;