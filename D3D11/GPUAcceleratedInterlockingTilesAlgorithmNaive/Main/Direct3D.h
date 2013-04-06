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

class D3DApplication
{
public:
    D3DApplication();
    virtual ~D3DApplication();

    inline float aspectRatio() const;

    int run(Direct3DData& direct3DData, 
            WindowState& windowState,
            WindowData& windowData);

    // Derived client class overrides these methods to 
    // implement specific application requirements.
    virtual bool init(Direct3DData& direct3DData, WindowData& windowData);
    virtual void onResize(Direct3DData& direct3DData); 
    virtual void updateScene(const float dt) = 0;
    virtual void drawScene(Direct3DData& direct3DData) = 0; 
    virtual LRESULT msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Convenience overrides for handling mouse input.
    virtual void onMouseDown(WPARAM btnState, 
                             const int32_t x, 
                             const int32_t y,
                             WindowData& windowData) = 0;

    virtual void onMouseUp(WPARAM btnState, 
                           const int32_t x, 
                           const int32_t y) = 0;

    virtual void onMouseMove(WPARAM btnState, 
                             const int32_t x, 
                             const int32_t y) = 0;

protected:
    bool initMainWindow(WindowData& windowData);
    bool initDirect3D(Direct3DData& direct3DData, WindowData& windowData);

    void calculateFrameStats(WindowData& windowData);

protected:
    Timer mTimer;

    // Derived class should set these in derived constructor to customize starting values.
    std::wstring mMainWindowCaption;
    uint32_t mClientWidth;
    uint32_t mClientHeight;
    uint32_t m4xMsaaQuality;
    bool mEnable4xMsaa;
};

inline float D3DApplication::aspectRatio() const
{
    return static_cast<float>(mClientWidth) / mClientHeight;
}