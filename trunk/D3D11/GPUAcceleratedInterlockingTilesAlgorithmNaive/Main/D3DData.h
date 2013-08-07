//////////////////////////////////////////////////////////////////////////
//
// Functions to initialize/destroy needed Direct3D structures
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <d3d11.h>

struct WindowData;

struct Direct3DData
{
    Direct3DData();

    ID3D11Device* mDevice;
    ID3D11DeviceContext* mImmediateContext;
    IDXGISwapChain* mSwapChain;
    ID3D11Texture2D* mDepthStencilBuffer;
    ID3D11RenderTargetView* mRenderTargetView;
    ID3D11DepthStencilView* mDepthStencilView;
    D3D11_VIEWPORT* mScreenViewport;
};

namespace Direct3DDataUtils
{
    bool init(Direct3DData& direct3DData, WindowData& windowData);
    void destroy(Direct3DData& direct3DData);
}