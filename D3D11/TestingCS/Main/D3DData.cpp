#include "D3DData.h"

#include <cassert>
#include <Camera.h>
#include <DxErrorChecker.h>

#include "Globals.h"

namespace
{
    bool createDeviceAndContext(ID3D11Device*& device,
                                ID3D11DeviceContext*& context,
                                const uint32_t createDeviceFlags)
    {
        D3D_FEATURE_LEVEL featureLevel;
        HRESULT result = D3D11CreateDevice(nullptr,           // default adapter
                                           D3D_DRIVER_TYPE_HARDWARE, 
                                           nullptr,           // no software device
                                           createDeviceFlags, 
                                           nullptr,           // feature level array 
                                           0,                 // feature level array size
                                           D3D11_SDK_VERSION,
                                           &device,
                                           &featureLevel,
                                           &context);

        // Check proper creation
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

        assert(device);
        assert(context);

        return true;
    }

    void fillSwapChainDesc(DXGI_SWAP_CHAIN_DESC& swapChainDesc,
                           const uint32_t windowWidth,
                           const uint32_t windowHeight,
                           const uint32_t sampleCount,
                           const DXGI_FORMAT displayFormat,
                           const uint32_t msaaQuality,
                           const HWND& mainWindow)
    {
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        
        // Backbuffer display mode
        swapChainDesc.BufferDesc.Width = windowWidth;
        swapChainDesc.BufferDesc.Height = windowHeight;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.Format = displayFormat;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        // Multi-sampling parameters
        swapChainDesc.SampleDesc.Count = sampleCount;
        swapChainDesc.SampleDesc.Quality = msaaQuality - 1;

        // Describe the surface usage and CPU access options for the back buffer
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

        // Number of buffers in the swap chain
        swapChainDesc.BufferCount = 1;
        swapChainDesc.OutputWindow = mainWindow;
        swapChainDesc.Windowed = true;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;
    }

    void createSwapChain(ID3D11Device& device,
                         DXGI_SWAP_CHAIN_DESC& swapChainDesc,
                         IDXGISwapChain*& swapChain)
    {
        // To correctly create the swap chain, we must use the IDXGIFactory that was
        // used to create the device."
        IDXGIDevice* dxgiDevice = nullptr;
        HRESULT result = device.QueryInterface(__uuidof(IDXGIDevice), 
                                               reinterpret_cast<void**> (&dxgiDevice));
        DxErrorChecker(result);  
        assert(dxgiDevice);

        IDXGIAdapter* dxgiAdapter = nullptr;
        result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**> (&dxgiAdapter));
        DxErrorChecker(result);  
        assert(dxgiAdapter);

        IDXGIFactory* dxgiFactory = nullptr;
        result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), 
                                        reinterpret_cast<void**> (&dxgiFactory));
        DxErrorChecker(result);
        assert(dxgiFactory);

        result = dxgiFactory->CreateSwapChain(&device, &swapChainDesc, &swapChain);
        DxErrorChecker(result);  
        assert(swapChain);

        dxgiDevice->Release();
        dxgiAdapter->Release();
        dxgiFactory->Release();
    }

    void fillDepthStencilBufferDesc(D3D11_TEXTURE2D_DESC& textureDesc,
                                    const uint32_t textureWidth,
                                    const uint32_t textureHeight,
                                    const DXGI_FORMAT& textureFormat,
                                    const uint32_t sampleCount,
                                    const uint32_t msaaQuality)
    {
        ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

        textureDesc.Width = textureWidth;
        textureDesc.Height = textureHeight;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = textureFormat;

        textureDesc.SampleDesc.Count = sampleCount;
        textureDesc.SampleDesc.Quality = msaaQuality - 1;

        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        textureDesc.CPUAccessFlags = 0; 
        textureDesc.MiscFlags = 0;
    }
}

namespace Direct3DDataUtils
{
    bool init(Direct3DData& direct3DData, WindowData& windowData)
    {
        assert(direct3DData.mDevice == nullptr);
        assert(direct3DData.mImmediateContext == nullptr);
        assert(direct3DData.mSwapChain == nullptr);
        assert(direct3DData.mRenderTargetView == nullptr);
        assert(direct3DData.mDepthStencilBuffer == nullptr);
        assert(direct3DData.mDepthStencilView == nullptr);
        assert(direct3DData.mScreenViewport == nullptr);

        //
        // Create device and device context
        //

        uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        ID3D11Device*& device = direct3DData.mDevice;
        ID3D11DeviceContext*& context = direct3DData.mImmediateContext;
        const bool success = createDeviceAndContext(device,
                                                    context,
                                                    createDeviceFlags);

        if (!success)
        {
            return false;
        }


        //
        // Compute MSAA quality
        //

        uint32_t& msaaQuality = windowData.m4xMsaaQuality;
        const uint32_t sampleCount = 4;
        const DXGI_FORMAT displayFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
        HRESULT result = device->CheckMultisampleQualityLevels(displayFormat,
                                                                     sampleCount,
                                                                     &msaaQuality);
        DxErrorChecker(result);
        assert(msaaQuality > 0);
        

        //
        // Create swap chain
        //
        
        // Initialize swap chain description
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        fillSwapChainDesc(swapChainDesc,
                          windowData.mClientWidth,
                          windowData.mClientHeight,
                          sampleCount,
                          displayFormat,
                          msaaQuality,
                          windowData.mMainWindow);

        IDXGISwapChain*& swapChain = direct3DData.mSwapChain;
        createSwapChain(*device, swapChainDesc, swapChain);


        //
        // Create render target view
        //
        ID3D11Texture2D* backBuffer = nullptr;
        result = swapChain->GetBuffer(0, // buffer number
                                      __uuidof(ID3D11Texture2D), 
                                      reinterpret_cast<void**>(&backBuffer));
        DxErrorChecker(result);
        assert(backBuffer);

        ID3D11RenderTargetView*& renderTargetView = direct3DData.mRenderTargetView;
        result = device->CreateRenderTargetView(backBuffer,
                                                nullptr, // render target view desc
                                                &renderTargetView);
        DxErrorChecker(result);
        assert(renderTargetView);
        backBuffer->Release();

        
        //
        // Create Depth Stencil Buffer and View
        //

        // Initialize depth stencil buffer
        D3D11_TEXTURE2D_DESC texture2DDesc;
        fillDepthStencilBufferDesc(texture2DDesc,
                                   windowData.mClientWidth,
                                   windowData.mClientHeight,
                                   DXGI_FORMAT_D24_UNORM_S8_UINT,
                                   sampleCount,
                                   msaaQuality);

        // Create depth stencil buffer
        ID3D11Texture2D*& depthStencilBuffer = direct3DData.mDepthStencilBuffer;
        result = device->CreateTexture2D(&texture2DDesc,
                                         nullptr, // sub resource data
                                         &depthStencilBuffer);

        DxErrorChecker(result);
        assert(depthStencilBuffer);

        // Create depth stencil view
        ID3D11DepthStencilView*& depthStencilView = direct3DData.mDepthStencilView;
        result = device->CreateDepthStencilView(depthStencilBuffer,
                                                nullptr, // depth stencil view desc
                                                &depthStencilView);

        DxErrorChecker(result);
        assert(depthStencilView);

        // Bind the render target view and depth/stencil view to the pipeline.
        context->OMSetRenderTargets(1,
                                    &renderTargetView, 
                                    depthStencilView);


        //
        // Init viewport
        //
        D3D11_VIEWPORT*& screenViewport = direct3DData.mScreenViewport;
        screenViewport = new D3D11_VIEWPORT();       
        screenViewport->TopLeftX = 0;
        screenViewport->TopLeftY = 0;
        screenViewport->Width = static_cast<float>(windowData.mClientWidth);
        screenViewport->Height = static_cast<float>(windowData.mClientHeight);
        screenViewport->MinDepth = 0.0f;
        screenViewport->MaxDepth = 1.0f;

        context->RSSetViewports(1, 
                                screenViewport);


        //
        // Initialize camera
        //

        const float aspectRatio = static_cast<float> (windowData.mClientWidth) 
                                  / windowData.mClientHeight;
        CameraUtils::setFrustrum(
            0.25f * DirectX::XM_PI, 
            aspectRatio, 
            1.0f, 
            1000.0f, 
            Globals::gCamera);

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