#pragma once

#include <cstdint>
#include <d3d11.h>

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

struct WindowData
{
    WindowData()
        : mClientWidth(0)
        , mClientHeight(0)
        , m4xMsaaQuality(0)
        , mWireframeMode(false)
    {
        ZeroMemory(&mMainWindow, sizeof(HWND));
    }

    HWND mMainWindow;
    uint32_t mClientWidth;
    uint32_t mClientHeight;
    uint32_t m4xMsaaQuality;
    bool mWireframeMode;
};

struct MouseProperties
{
    POINT mLastPosition;
};

namespace WindowDataUtils
{
    bool init(WindowData& windowData);
}

namespace Events
{
    void onMouseMove(WPARAM btnState, 
                     const int32_t x, 
                     const int32_t y,
                     MouseProperties& mouseProperties);

    void onMouseDown(WPARAM btnState, 
                     const int32_t x,
                     const int32_t y,
                     MouseProperties& mouseProperties);

    void onMouseUp(WPARAM btnState, 
                   const int32_t x,
                   const int32_t y,
                   MouseProperties& mouseProperties);
}