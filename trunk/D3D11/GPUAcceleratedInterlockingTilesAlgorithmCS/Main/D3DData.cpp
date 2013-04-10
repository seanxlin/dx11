#include "D3DData.h"

#include <cassert>
#include <DxErrorChecker.h>

#include "WindowManager.h"

namespace Direct3DDataUtils
{
    bool init(Direct3DData& direct3DData, WindowData& windowData)
    {
        // Create the device and device context.
        uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        assert(direct3DData.mDevice == nullptr);
        ID3D11Device*& device = direct3DData.mDevice;

        assert(direct3DData.mImmediateContext == nullptr);
        ID3D11DeviceContext*& context = direct3DData.mImmediateContext;

        D3D_FEATURE_LEVEL featureLevel;
        HRESULT result = D3D11CreateDevice(
            0,                 // default adapter
            D3D_DRIVER_TYPE_HARDWARE,
            0,                 // no software device
            createDeviceFlags, 
            0, 0,              // default feature level array
            D3D11_SDK_VERSION,
            &device,
            &featureLevel,
            &context);

        assert(device);
        assert(context);

        if (result < 0)
        {
            MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);

            return false;
        }

        if (featureLevel != D3D_FEATURE_LEVEL_11_0)
        {
            MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);

            return false;
        }

        // Check 4X MSAA quality support for our back buffer format.
        // All Direct3D 11 capable devices support 4X MSAA for all render 
        // target formats, so we only need to check quality support.
        uint32_t& msaaQuality = windowData.m4xMsaaQuality;
        result = device->CheckMultisampleQualityLevels(
            DXGI_FORMAT_R8G8B8A8_UNORM, 
            4, 
            &msaaQuality);

        DxErrorChecker(result);

        assert(msaaQuality > 0);

        // Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
        const uint32_t windowWidth = windowData.mClientWidth;
        const uint32_t windowHeight = windowData.mClientHeight;
        DXGI_SWAP_CHAIN_DESC swapChainDescription;
        swapChainDescription.BufferDesc.Width = windowWidth;
        swapChainDescription.BufferDesc.Height = windowHeight;
        swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        // Use 4X MSAA? 
        if (windowData.mEnable4xMsaa)
        {
            swapChainDescription.SampleDesc.Count = 4;
            swapChainDescription.SampleDesc.Quality = msaaQuality - 1;
        }
        // No MSAA
        else
        {
            swapChainDescription.SampleDesc.Count = 1;
            swapChainDescription.SampleDesc.Quality = 0;
        }

        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.BufferCount = 1;
        swapChainDescription.OutputWindow = windowData.mMainWindow;
        swapChainDescription.Windowed = true;
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDescription.Flags = 0;

        // To correctly create the swap chain, we must use the IDXGIFactory that was
        // used to create the device. If we tried to use a different IDXGIFactory instance
        // (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
        // This function is being called with a device from a different IDXGIFactory."
        IDXGIDevice* dxgiDevice = nullptr;
        result = device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**> (&dxgiDevice));
        DxErrorChecker(result);  

        IDXGIAdapter* dxgiAdapter = nullptr;
        result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**> (&dxgiAdapter));
        DxErrorChecker(result);  

        IDXGIFactory* dxgiFactory = nullptr;
        result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**> (&dxgiFactory));
        DxErrorChecker(result);  

        result = dxgiFactory->CreateSwapChain(device, &swapChainDescription, &direct3DData.mSwapChain);
        DxErrorChecker(result);  

        dxgiDevice->Release();
        dxgiAdapter->Release();
        dxgiFactory->Release();

        // The remaining steps that need to be carried out for d3d creation
        // also need to be executed every time the window is resized. So
        // just call the OnResize method here to avoid code duplication.	
        Events::onResize();

        return true;
    }

    void destroy(Direct3DData& direct3DData)
    {
        direct3DData.mRenderTargetView->Release();
        direct3DData.mDepthStencilView->Release();
        direct3DData.mSwapChain->Release();
        direct3DData.mDepthStencilBuffer->Release();

        // Restore all default settings.
        direct3DData.mImmediateContext->ClearState();

        direct3DData.mImmediateContext->Release();
        direct3DData.mDevice->Release();
        delete direct3DData.mScreenViewport;
    }
}