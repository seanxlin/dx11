#include "D3DApplication.h"

#include <cassert>
#include <sstream>
#include <WindowsX.h>

#include <D3DErrorChecker.h>

namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	Framework::D3DApplication* gD3DApp = 0;
}

namespace Framework
{
    LRESULT CALLBACK
        MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Forward hwnd on because we can get messages (e.g., WM_CREATE)
        // before CreateWindow returns, and thus before mhMainWnd is valid.
        return gD3DApp->msgProc(hwnd, msg, wParam, lParam);
    }

    D3DApplication::D3DApplication(HINSTANCE hInstance)
        : mAppInstance(hInstance)
        , mMainWindow(0)
        , mDevice(nullptr)
        , mImmediateContext(nullptr)
        , mSwapChain(nullptr)
        , mDepthStencilBuffer(nullptr)
        , mRenderTargetView(nullptr)
        , mDepthStencilView(nullptr)
        , mDriverType(D3D_DRIVER_TYPE_HARDWARE)
        , mMainWindowCaption(L"D3D11 Application")
        , mClientWidth(800)
        , mClientHeight(600)
        , m4xMsaaQuality(0)
        , mIsPaused(false)
        , mIsMinimized(false)
        , mIsMaximized(false)
        , mIsResizing(false)
        , mEnable4xMsaa(false)
    {
        ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));

        // Get a pointer to the application object so we can forward 
        // Windows messages to the object's window procedure through
        // the global window procedure.
        gD3DApp = this;
    }

    D3DApplication::~D3DApplication()
    {
        mRenderTargetView->Release();
        mDepthStencilView->Release();
        mSwapChain->Release();
        mDepthStencilBuffer->Release();

        // Restore all default settings.
        mImmediateContext->ClearState();

        mImmediateContext->Release();
        mDevice->Release();
    }

    int D3DApplication::run()
    {
        MSG msg = {0};

        mTimer.reset();

        while(msg.message != WM_QUIT)
        {
            // If there are Window messages then process them.
            if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
            // Otherwise, do animation/game stuff.
            else
            {	
                mTimer.tick();

                if( !mIsPaused )
                {
                    calculateFrameStats();
                    updateScene(mTimer.deltaTime());	
                    drawScene();
                }
                else
                {
                    Sleep(100);
                }
            }
        }

        return static_cast<int> (msg.wParam);
    }

    bool D3DApplication::init()
    {
        return initMainWindow() && initDirect3D();
    }

    void D3DApplication::onResize()
    {
        assert(mImmediateContext);
        assert(mDevice);
        assert(mSwapChain);

        // Release the old views, as they hold references to the buffers we
        // will be destroying. Also release the old depth/stencil buffer.
        if (mRenderTargetView)
            mRenderTargetView->Release();

        if (mDepthStencilView)
            mDepthStencilView->Release();

        if (mDepthStencilBuffer)
            mDepthStencilBuffer->Release();


        // Resize the swap chain and recreate the render target view.
        HRESULT result = mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        DebugUtils::ErrorChecker(result);

        ID3D11Texture2D* backBuffer = nullptr;
        result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        DebugUtils::ErrorChecker(result);

        result  = mDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView);
        DebugUtils::ErrorChecker(result);
        backBuffer->Release();

        // Create the depth/stencil buffer and view.
        D3D11_TEXTURE2D_DESC depthStencilDesc;	
        depthStencilDesc.Width = mClientWidth;
        depthStencilDesc.Height = mClientHeight;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        // Use 4X MSAA? --must match swap chain MSAA values.
        if (mEnable4xMsaa)
        {
            depthStencilDesc.SampleDesc.Count = 4;
            depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
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

        result = mDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer);
        DebugUtils::ErrorChecker(result);

        result = mDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);
        DebugUtils::ErrorChecker(result);

        // Bind the render target view and depth/stencil view to the pipeline.
        mImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

        // Set the viewport transform.
        mScreenViewport.TopLeftX = 0;
        mScreenViewport.TopLeftY = 0;
        mScreenViewport.Width = static_cast<float>(mClientWidth);
        mScreenViewport.Height = static_cast<float>(mClientHeight);
        mScreenViewport.MinDepth = 0.0f;
        mScreenViewport.MaxDepth = 1.0f;

        mImmediateContext->RSSetViewports(1, &mScreenViewport);
    }

    LRESULT D3DApplication::msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            // WM_ACTIVATE is sent when the window is activated or deactivated.  
            // We pause the game when the window is deactivated and unpause it 
            // when it becomes active.  
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                mIsPaused = true;
                mTimer.stop();
            }
            else
            {
                mIsPaused = false;
                mTimer.start();
            }

            return 0;

            // WM_SIZE is sent when the user resizes the window.  
        case WM_SIZE:
            // Save the new client area dimensions.
            mClientWidth = LOWORD(lParam);
            mClientHeight = HIWORD(lParam);
            if (mDevice)
            {
                if (wParam == SIZE_MINIMIZED)
                {
                    mIsPaused = true;
                    mIsMinimized = true;
                    mIsMaximized = false;
                }
                else if (wParam == SIZE_MAXIMIZED)
                {
                    mIsPaused = false;
                    mIsMinimized = false;
                    mIsMaximized = true;
                    onResize();
                }
                else if (wParam == SIZE_RESTORED)
                {				
                    // Restoring from minimized state?
                    if (mIsMinimized)
                    {
                        mIsPaused = false;
                        mIsMinimized = false;
                        onResize();
                    }

                    // Restoring from maximized state?
                    else if (mIsMaximized)
                    {
                        mIsPaused = false;
                        mIsMaximized = false;
                        onResize();
                    }

                    else if (mIsResizing)
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
                        onResize();
                }
            }

            return 0;

            // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
        case WM_ENTERSIZEMOVE:
            mIsPaused = true;
            mIsResizing  = true;
            mTimer.stop();
            return 0;

            // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
            // Here we reset everything based on the new window dimensions.
        case WM_EXITSIZEMOVE:
            mIsPaused = false;
            mIsResizing  = false;
            mTimer.start();
            onResize();
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
            onMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            onMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_MOUSEMOVE:
            onMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    bool D3DApplication::initMainWindow()
    {
        WNDCLASS windowClass;
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = MainWndProc; 
        windowClass.cbClsExtra = 0;
        windowClass.cbWndExtra = 0;
        windowClass.hInstance = mAppInstance;
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

        // Compute window rectangle dimensions based on requested client area dimensions.
        RECT rect = { 0, 0, mClientWidth, mClientHeight };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
        const uint32_t width = rect.right - rect.left;
        const uint32_t height = rect.bottom - rect.top;

        mMainWindow = CreateWindow(L"D3DWndClassName", mMainWindowCaption.c_str(), 
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mAppInstance, 0); 
        if (!mMainWindow)
        {
            MessageBox(0, L"CreateWindow Failed.", 0, 0);

            return false;
        }

        ShowWindow(mMainWindow, SW_SHOW);
        UpdateWindow(mMainWindow);

        return true;
    }

    bool D3DApplication::initDirect3D()
    {
        // Create the device and device context.
        uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevel;
        HRESULT result = D3D11CreateDevice(
            0,                 // default adapter
            mDriverType,
            0,                 // no software device
            createDeviceFlags, 
            0, 0,              // default feature level array
            D3D11_SDK_VERSION,
            &mDevice,
            &featureLevel,
            &mImmediateContext);

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
        result = mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
        DebugUtils::ErrorChecker(result);

        assert(m4xMsaaQuality > 0);

        // Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
        DXGI_SWAP_CHAIN_DESC swapChainDescription;
        swapChainDescription.BufferDesc.Width = mClientWidth;
        swapChainDescription.BufferDesc.Height = mClientHeight;
        swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        // Use 4X MSAA? 
        if (mEnable4xMsaa)
        {
            swapChainDescription.SampleDesc.Count = 4;
            swapChainDescription.SampleDesc.Quality = m4xMsaaQuality - 1;
        }
        // No MSAA
        else
        {
            swapChainDescription.SampleDesc.Count = 1;
            swapChainDescription.SampleDesc.Quality = 0;
        }

        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.BufferCount = 1;
        swapChainDescription.OutputWindow = mMainWindow;
        swapChainDescription.Windowed = true;
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDescription.Flags = 0;

        // To correctly create the swap chain, we must use the IDXGIFactory that was
        // used to create the device. If we tried to use a different IDXGIFactory instance
        // (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
        // This function is being called with a device from a different IDXGIFactory."
        IDXGIDevice* dxgiDevice = nullptr;
        result = mDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**> (&dxgiDevice));
        DebugUtils::ErrorChecker(result);  

        IDXGIAdapter* dxgiAdapter = nullptr;
        result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**> (&dxgiAdapter));
        DebugUtils::ErrorChecker(result);  

        IDXGIFactory* dxgiFactory = nullptr;
        result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**> (&dxgiFactory));
        DebugUtils::ErrorChecker(result);  

        result = dxgiFactory->CreateSwapChain(mDevice, &swapChainDescription, &mSwapChain);
        DebugUtils::ErrorChecker(result);  

        dxgiDevice->Release();
        dxgiAdapter->Release();
        dxgiFactory->Release();

        // The remaining steps that need to be carried out for d3d creation
        // also need to be executed every time the window is resized. So
        // just call the OnResize method here to avoid code duplication.	
        onResize();

        return true;
    }

    void D3DApplication::calculateFrameStats()
    {
        // Code computes the average frames per second, and also the 
        // average time it takes to render one frame.  These stats 
        // are appended to the window caption bar.
        static uint32_t frameCounter = 0;
        static float timeElapsed = 0.0f;

        ++frameCounter;

        // Compute averages over one second period.
        if( (mTimer.inGameTime() - timeElapsed) >= 1.0f )
        {
            const float fps = static_cast<float> (frameCounter); // fps = frameCnt / 1
            const float mspf = 1000.0f / fps;

            std::wostringstream outs;   
            outs.precision(6);
            outs << mMainWindowCaption << L"    "
                << L"FPS: " << fps << L"    " 
                << L"Frame Time: " << mspf << L" (ms)";

            SetWindowText(mMainWindow, outs.str().c_str());

            // Reset for next average.
            frameCounter = 0;
            timeElapsed += 1.0f;
        }
    }
}