#include "WindowManager.h"

#include <d3d11.h>
#include <WindowsX.h>

#include <DxErrorChecker.h>
#include <Timer.h>

#include "Globals.h"

namespace
{
    LRESULT msgProc(HWND hwnd, 
                    UINT msg, 
                    WPARAM wParam,
                    LPARAM lParam)
    {
        switch (msg)
        {
            // WM_ACTIVATE is sent when the window is activated or deactivated.  
            // We pause the game when the window is deactivated and unpause it 
            // when it becomes active.  
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                Globals::gWindowState.mIsPaused = true;
                TimerUtils::stop(Globals::gTimer);
            }
            else
            {
                Globals::gWindowState.mIsPaused = false;
                TimerUtils::start(Globals::gTimer);
            }

            return 0;

            // WM_SIZE is sent when the user resizes the window.  
        case WM_SIZE:
            if (Globals::gDirect3DData.mDevice) 
            {
                // Save the new client area dimensions.
                Globals::gWindowData.mClientWidth = LOWORD(lParam);
                Globals::gWindowData.mClientHeight = HIWORD(lParam);

                if (wParam == SIZE_MINIMIZED)
                {
                    Globals::gWindowState.mIsPaused = true;
                    Globals::gWindowState.mIsMinimized = true;
                    Globals::gWindowState.mIsMaximized = false;
                }
                else if (wParam == SIZE_MAXIMIZED)
                {
                    Globals::gWindowState.mIsPaused = false;
                    Globals::gWindowState.mIsMinimized = false;
                    Globals::gWindowState.mIsMaximized = true;
                    Events::onResize();
                }
                else if (wParam == SIZE_RESTORED)
                {				
                    // Restoring from minimized state?
                    if (Globals::gWindowState.mIsMinimized)
                    {
                        Globals::gWindowState.mIsPaused = false;
                        Globals::gWindowState.mIsMinimized = false;
                        Events::onResize();
                    }

                    // Restoring from maximized state?
                    else if (Globals::gWindowState.mIsMaximized)
                    {
                        Globals::gWindowState.mIsPaused = false;
                        Globals::gWindowState.mIsMaximized = false;
                        Events::onResize();
                    }

                    else if (Globals::gWindowState.mIsResizing)
                    {
                        // If user is dragging the resize bars, we do not resize 
                        // the buffers here because as the user continuously 
                        // drags the resize bars, a stream of WM_SIZE messages are
                        // sent to the window, and it would be pointless (and slow)
                        // to resize for each WM_SIZE message received from dragging
                        // the resize bars.  So instead, we reset after the user is 
                        // done resizing the window and releases the resize bars, which 
                        // sends a WM_EXITSIZEMOVE message.
                    }

                    else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                        Events::onResize();
                }
            }
 
            return 0;

            // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
        case WM_ENTERSIZEMOVE:
            Globals::gWindowState.mIsPaused = true;
            Globals::gWindowState.mIsResizing  = true;
            TimerUtils::stop(Globals::gTimer);
            return 0;

            // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
            // Here we reset everything based on the new window dimensions.
        case WM_EXITSIZEMOVE:
            Globals::gWindowState.mIsPaused = false;
            Globals::gWindowState.mIsResizing  = false;
            TimerUtils::start(Globals::gTimer);
            Events::onResize();
            return 0;

            // WM_DESTROY is sent when the window is being destroyed.
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

            // The WM_MENUCHAR message is sent when a menu is active and the user presses 
            // a key that does not correspond to any mnemonic or accelerator key. 
        case WM_MENUCHAR:
            // Don't beep when we alt-enter.
            return MAKELRESULT(0, MNC_CLOSE);

            // Catch this message so to prevent the window from becoming too small.
        case WM_GETMINMAXINFO:
            (reinterpret_cast<MINMAXINFO*> (lParam))->ptMinTrackSize.x = 200;
            (reinterpret_cast<MINMAXINFO*> (lParam))->ptMinTrackSize.y = 200; 
            return 0;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            Events::onMouseDown(
                wParam, 
                GET_X_LPARAM(lParam), 
                GET_Y_LPARAM(lParam),
                Globals::gMouseProperties);
            return 0;

        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            Events::onMouseUp(
                wParam, 
                GET_X_LPARAM(lParam), 
                GET_Y_LPARAM(lParam),
                Globals::gMouseProperties);
            return 0;

        case WM_MOUSEMOVE:
            Events::onMouseMove(
                wParam, 
                GET_X_LPARAM(lParam), 
                GET_Y_LPARAM(lParam),
                Globals::gMouseProperties);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

LRESULT CALLBACK MainWndProc(HWND hwnd, 
                             UINT msg, 
                             WPARAM wParam, 
                             LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return msgProc(hwnd, msg, wParam, lParam);
}

namespace WindowDataUtils
{
    bool init(WindowData& windowData)
    {
        const HINSTANCE& appInstance = Globals::gAppInstance;

        WNDCLASS windowClass;
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = MainWndProc; 
        windowClass.cbClsExtra = 0;
        windowClass.cbWndExtra = 0;
        windowClass.hInstance = appInstance;
        windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
        windowClass.hCursor = LoadCursor(0, IDC_ARROW);
        windowClass.hbrBackground = reinterpret_cast<HBRUSH> (GetStockObject(NULL_BRUSH));
        windowClass.lpszMenuName = 0;
        windowClass.lpszClassName = L"D3DWndClassName";

        if (!RegisterClass(&windowClass))
        {
            MessageBox(0, L"RegisterClass Failed.", 0, 0);

            return false;
        }

        // Init window size
        windowData.mClientWidth = 800;
        windowData.mClientHeight = 600;

        // Compute window rectangle dimensions based on requested client area dimensions.
        RECT rect = 
        { 
            0, 
            0, 
            windowData.mClientWidth, 
            windowData.mClientHeight 
        };
        AdjustWindowRect(
            &rect, 
            WS_OVERLAPPEDWINDOW, 
            false);
        const uint32_t width = rect.right - rect.left;
        const uint32_t height = rect.bottom - rect.top;

        windowData.mMainWindow = CreateWindow(
            L"D3DWndClassName", 
            L"GPU Accelerated Interlocking Tiles Algorithm Naive Demo", 
            WS_OVERLAPPEDWINDOW, 
            CW_USEDEFAULT, 
            CW_USEDEFAULT, 
            width, 
            height, 
            0, 
            0, 
            appInstance, 
            0); 
        if (!windowData.mMainWindow)
        {
            MessageBox(0, L"CreateWindow Failed.", 0, 0);

            return false;
        }

        ShowWindow(windowData.mMainWindow, SW_SHOW);
        UpdateWindow(windowData.mMainWindow);

        return true;
    }
}

namespace Events
{
    void onResize()
    {
        assert(Globals::gDirect3DData.mImmediateContext);
        assert(Globals::gDirect3DData.mDevice);
        assert(Globals::gDirect3DData.mSwapChain);

        // Release the old views, as they hold references to the buffers we
        // will be destroying. Also release the old depth/stencil buffer.
        if (Globals::gDirect3DData.mRenderTargetView)
        {
            Globals::gDirect3DData.mRenderTargetView->Release();
        }

        if (Globals::gDirect3DData.mDepthStencilView)
        {
            Globals::gDirect3DData.mDepthStencilView->Release();
        }

        if (Globals::gDirect3DData.mDepthStencilBuffer)
        {
            Globals::gDirect3DData.mDepthStencilBuffer->Release();
        }

        // Resize the swap chain and recreate the render target view.
        HRESULT result = Globals::gDirect3DData.mSwapChain->ResizeBuffers(
            1, 
            Globals::gWindowData.mClientWidth, 
            Globals::gWindowData.mClientHeight, 
            DXGI_FORMAT_R8G8B8A8_UNORM, 0);

        DxErrorChecker(result);

        ID3D11Texture2D* backBuffer = nullptr;
        result = Globals::gDirect3DData.mSwapChain->GetBuffer(
            0, 
            __uuidof(ID3D11Texture2D), 
            reinterpret_cast<void**>(&backBuffer));

        DxErrorChecker(result);

        result = Globals::gDirect3DData.mDevice->CreateRenderTargetView(
            backBuffer, 
            0, 
            &Globals::gDirect3DData.mRenderTargetView);

        DxErrorChecker(result);
        backBuffer->Release();

        // Create the depth/stencil buffer and view.
        D3D11_TEXTURE2D_DESC depthStencilDesc;	
        depthStencilDesc.Width = Globals::gWindowData.mClientWidth;
        depthStencilDesc.Height = Globals::gWindowData.mClientHeight;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        // Use 4X MSAA? --must match swap chain MSAA values.
        if (Globals::gWindowData.mEnable4xMsaa)
        {
            depthStencilDesc.SampleDesc.Count = 4;
            depthStencilDesc.SampleDesc.Quality = Globals::gWindowData.m4xMsaaQuality - 1;
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

        result = Globals::gDirect3DData.mDevice->CreateTexture2D(
            &depthStencilDesc, 
            0, 
            &Globals::gDirect3DData.mDepthStencilBuffer);

        DxErrorChecker(result);

        result = Globals::gDirect3DData.mDevice->CreateDepthStencilView(
            Globals::gDirect3DData.mDepthStencilBuffer,
            0, 
            &Globals::gDirect3DData.mDepthStencilView);

        DxErrorChecker(result);

        // Bind the render target view and depth/stencil view to the pipeline.
        Globals::gDirect3DData.mImmediateContext->OMSetRenderTargets(
            1, 
            &Globals::gDirect3DData.mRenderTargetView, 
            Globals::gDirect3DData.mDepthStencilView);

        // Set the viewport transform.
        if (!Globals::gDirect3DData.mScreenViewport) {
            Globals::gDirect3DData.mScreenViewport = new D3D11_VIEWPORT();
        }

        Globals::gDirect3DData.mScreenViewport->TopLeftX = 0;
        Globals::gDirect3DData.mScreenViewport->TopLeftY = 0;
        Globals::gDirect3DData.mScreenViewport->Width = static_cast<float>(Globals::gWindowData.mClientWidth);
        Globals::gDirect3DData.mScreenViewport->Height = static_cast<float>(Globals::gWindowData.mClientHeight);
        Globals::gDirect3DData.mScreenViewport->MinDepth = 0.0f;
        Globals::gDirect3DData.mScreenViewport->MaxDepth = 1.0f;

        Globals::gDirect3DData.mImmediateContext->RSSetViewports(1, Globals::gDirect3DData.mScreenViewport);

        const float aspectRatio = static_cast<float> (Globals::gWindowData.mClientWidth) / Globals::gWindowData.mClientHeight;
        CameraUtils::setFrustrum(
            0.25f * DirectX::XM_PI, 
            aspectRatio, 
            1.0f, 
            1000.0f, 
            Globals::gCamera);
    }

    void onMouseDown(WPARAM btnState, 
                     const int32_t x,
                     const int32_t y,
                     MouseProperties& mouseProperties)
    {
        mouseProperties.mLastPosition.x = x;   
        mouseProperties.mLastPosition.y = y;

        SetCapture(Globals::gWindowData.mMainWindow);
    }

    void onMouseUp(WPARAM btnState, 
                   const int32_t x,
                   const int32_t y,
                   MouseProperties& mouseProperties)
    {
        mouseProperties.mLastPosition.x = x;   
        mouseProperties.mLastPosition.y = y;

        ReleaseCapture();
    }

    void onMouseMove(WPARAM btnState, 
                     const int32_t x, 
                     const int32_t y,
                     MouseProperties& mouseProperties)
    {
        if ((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.15f * static_cast<float>(x - mouseProperties.mLastPosition.x));
            const float dy = DirectX::XMConvertToRadians(0.15f * static_cast<float>(y - mouseProperties.mLastPosition.y));

            CameraUtils::pitch(dy, Globals::gCamera);
            CameraUtils::rotateY(dx, Globals::gCamera);
        }

        mouseProperties.mLastPosition.x = x;
        mouseProperties.mLastPosition.y = y;
    }
}