#include "WindowManager.h"

#include <windowsx.h>

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
        switch (msg) {
            // WM_ACTIVATE is sent when the window is activated or deactivated.  
            // We pause the game when the window is deactivated and unpause it 
            // when it becomes active.  
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                Globals::gWindowState.mIsPaused = true;
                TimerUtils::stop(Globals::gTimer);
            } else {
                Globals::gWindowState.mIsPaused = false;
                TimerUtils::start(Globals::gTimer);
            }

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
            Events::onMouseDown(wParam,
                                GET_X_LPARAM(lParam),
                                GET_Y_LPARAM(lParam),
                                Globals::gMouseProperties);

            return 0;

        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            Events::onMouseUp(wParam,
                              GET_X_LPARAM(lParam),
                              GET_Y_LPARAM(lParam),
                              Globals::gMouseProperties);

            return 0;

        case WM_MOUSEMOVE:
            Events::onMouseMove(wParam, 
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

        // Set window class properties
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

        // Register window class
        if (!RegisterClass(&windowClass)) {
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
        AdjustWindowRect(&rect, 
                         WS_OVERLAPPEDWINDOW,
                         false);
        const uint32_t width = rect.right - rect.left;
        const uint32_t height = rect.bottom - rect.top;

        // Create window
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
            0
        ); 
        if (!windowData.mMainWindow) {
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
    void onMouseDown(WPARAM buttonState, 
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
        if ((btnState & MK_LBUTTON) != 0) {
            // Make each pixel correspond to a quarter of a degree.
            const float deltaX = static_cast<float>(x - mouseProperties.mLastPosition.x);
            const float deltaY = static_cast<float>(y - mouseProperties.mLastPosition.y);
            const float factor = 0.15f;
            const float deltaXInRadians = DirectX::XMConvertToRadians(factor * deltaX);
            const float deltaYInRadians = DirectX::XMConvertToRadians(factor * deltaY);

            CameraUtils::pitch(deltaYInRadians, Globals::gCamera);
            CameraUtils::rotateAboutYAxis(deltaXInRadians, Globals::gCamera);
        }

        mouseProperties.mLastPosition.x = x;
        mouseProperties.mLastPosition.y = y;
    }
}