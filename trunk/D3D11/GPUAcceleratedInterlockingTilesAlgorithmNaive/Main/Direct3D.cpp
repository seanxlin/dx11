#include "Direct3D.h"

#include <cassert>
#include <sstream>
#include <WindowsX.h>

#include <DxErrorChecker.h>

namespace
{
    // This is just used to forward Windows messages from a global window
    // procedure to our member function window procedure because we cannot
    // assign a member function to WNDCLASS::lpfnWndProc.
    D3DApplication* gD3DApp = 0;
}

LRESULT CALLBACK
    MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return gD3DApp->msgProc(hwnd, msg, wParam, lParam);
}

D3DApplication::D3DApplication()
    : mMainWindowCaption(L"D3D11 Application")
    , mClientWidth(800)
    , mClientHeight(600)
    , m4xMsaaQuality(0)
    , mEnable4xMsaa(false)
{

    // Get a pointer to the application object so we can forward 
    // Windows messages to the object's window procedure through
    // the global window procedure.
    gD3DApp = this;
}

D3DApplication::~D3DApplication()
{

}

int D3DApplication::run(Direct3DData& direct3DData, 
                        WindowState& windowState,
                        WindowData& windowData)
{
    MSG msg = {0};

    TimerUtils::reset(mTimer);

    while (msg.message != WM_QUIT)
    {
        // If there are Window messages then process them.
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Otherwise, do animation/game stuff.
        else
        {	
            TimerUtils::tick(mTimer);

            if (!windowState.mIsPaused)
            {
                calculateFrameStats(windowData);
                updateScene(static_cast<float> (mTimer.mDeltaTime));	
                drawScene(direct3DData);
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return static_cast<int> (msg.wParam);
}

bool D3DApplication::init(Direct3DData& direct3DData, WindowData& windowData)
{
    return initMainWindow(windowData) && initDirect3D(direct3DData, windowData);
}

void D3DApplication::onResize(Direct3DData& direct3DData)
{
    assert(direct3DData.mImmediateContext);
    assert(direct3DData.mDevice);
    assert(direct3DData.mSwapChain);

    // Release the old views, as they hold references to the buffers we
    // will be destroying. Also release the old depth/stencil buffer.
    if (direct3DData.mRenderTargetView)
    {
        direct3DData.mRenderTargetView->Release();
    }

    if (direct3DData.mDepthStencilView)
    {
        direct3DData.mDepthStencilView->Release();
    }

    if (direct3DData.mDepthStencilBuffer)
    {
        direct3DData.mDepthStencilBuffer->Release();
    }

    // Resize the swap chain and recreate the render target view.
    HRESULT result = direct3DData.mSwapChain->ResizeBuffers(1, 
                                                            mClientWidth, 
                                                            mClientHeight, 
                                                            DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    DxErrorChecker(result);

    ID3D11Texture2D* backBuffer = nullptr;
    result = direct3DData.mSwapChain->GetBuffer(0, 
                                                __uuidof(ID3D11Texture2D), 
                                                reinterpret_cast<void**>(&backBuffer));
    DxErrorChecker(result);

    result = direct3DData.mDevice->CreateRenderTargetView(backBuffer, 
                                                          0, 
                                                          &direct3DData.mRenderTargetView);
    DxErrorChecker(result);
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

    result = direct3DData.mDevice->CreateTexture2D(&depthStencilDesc, 
                                                   0, 
                                                   &direct3DData.mDepthStencilBuffer);
    DxErrorChecker(result);

    result = direct3DData.mDevice->CreateDepthStencilView(direct3DData.mDepthStencilBuffer,
                                                          0, 
                                                          &direct3DData.mDepthStencilView);
    DxErrorChecker(result);

    // Bind the render target view and depth/stencil view to the pipeline.
    direct3DData.mImmediateContext->OMSetRenderTargets(1, 
                                                       &direct3DData.mRenderTargetView, 
                                                       direct3DData.mDepthStencilView);

    // Set the viewport transform.
    direct3DData.mScreenViewport.TopLeftX = 0;
    direct3DData.mScreenViewport.TopLeftY = 0;
    direct3DData.mScreenViewport.Width = static_cast<float>(mClientWidth);
    direct3DData.mScreenViewport.Height = static_cast<float>(mClientHeight);
    direct3DData.mScreenViewport.MinDepth = 0.0f;
    direct3DData.mScreenViewport.MaxDepth = 1.0f;

    direct3DData.mImmediateContext->RSSetViewports(1, &direct3DData.mScreenViewport);
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
            gWindowState.mIsPaused = true;
            TimerUtils::stop(mTimer);
        }
        else
        {
            gWindowState.mIsPaused = false;
            TimerUtils::start(mTimer);
        }

        return 0;

        // WM_SIZE is sent when the user resizes the window.  
    case WM_SIZE:
        // Save the new client area dimensions.
        mClientWidth = LOWORD(lParam);
        mClientHeight = HIWORD(lParam);
        if (gDirect3DData.mDevice)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                gWindowState.mIsPaused = true;
                gWindowState.mIsMinimized = true;
                gWindowState.mIsMaximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                gWindowState.mIsPaused = false;
                gWindowState.mIsMinimized = false;
                gWindowState.mIsMaximized = true;
                onResize(gDirect3DData);
            }
            else if (wParam == SIZE_RESTORED)
            {				
                // Restoring from minimized state?
                if (gWindowState.mIsMinimized)
                {
                    gWindowState.mIsPaused = false;
                    gWindowState.mIsMinimized = false;
                    onResize(gDirect3DData);
                }

                // Restoring from maximized state?
                else if (gWindowState.mIsMaximized)
                {
                    gWindowState.mIsPaused = false;
                    gWindowState.mIsMaximized = false;
                    onResize(gDirect3DData);
                }

                else if (gWindowState.mIsResizing)
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
                    onResize(gDirect3DData);
            }
        }

        return 0;

        // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
    case WM_ENTERSIZEMOVE:
        gWindowState.mIsPaused = true;
        gWindowState.mIsResizing  = true;
        TimerUtils::stop(mTimer);
        return 0;

        // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
        // Here we reset everything based on the new window dimensions.
    case WM_EXITSIZEMOVE:
        gWindowState.mIsPaused = false;
        gWindowState.mIsResizing  = false;
        TimerUtils::start(mTimer);
        onResize(gDirect3DData);
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
        onMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), gWindowData);
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

bool D3DApplication::initMainWindow(WindowData& windowData)
{
    WNDCLASS windowClass;
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWndProc; 
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = windowData.mAppInstance;
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

    windowData.mMainWindow = CreateWindow(L"D3DWndClassName", 
                                          mMainWindowCaption.c_str(), 
                                          WS_OVERLAPPEDWINDOW, 
                                          CW_USEDEFAULT, 
                                          CW_USEDEFAULT, 
                                          width, 
                                          height, 
                                          0, 
                                          0, 
                                          windowData.mAppInstance, 
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

bool D3DApplication::initDirect3D(Direct3DData& direct3DData, WindowData& windowData)
{
    // Create the device and device context.
    uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT result = D3D11CreateDevice(
        0,                 // default adapter
        direct3DData.mDriverType,
        0,                 // no software device
        createDeviceFlags, 
        0, 0,              // default feature level array
        D3D11_SDK_VERSION,
        &direct3DData.mDevice,
        &featureLevel,
        &direct3DData.mImmediateContext);

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
    result = direct3DData.mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 
                                                                 4, 
                                                                 &m4xMsaaQuality);
    DxErrorChecker(result);

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
    swapChainDescription.OutputWindow = windowData.mMainWindow;
    swapChainDescription.Windowed = true;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDescription.Flags = 0;

    // To correctly create the swap chain, we must use the IDXGIFactory that was
    // used to create the device. If we tried to use a different IDXGIFactory instance
    // (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
    // This function is being called with a device from a different IDXGIFactory."
    IDXGIDevice* dxgiDevice = nullptr;
    result = direct3DData.mDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**> (&dxgiDevice));
    DxErrorChecker(result);  

    IDXGIAdapter* dxgiAdapter = nullptr;
    result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**> (&dxgiAdapter));
    DxErrorChecker(result);  

    IDXGIFactory* dxgiFactory = nullptr;
    result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**> (&dxgiFactory));
    DxErrorChecker(result);  

    result = dxgiFactory->CreateSwapChain(direct3DData.mDevice, &swapChainDescription, &direct3DData.mSwapChain);
    DxErrorChecker(result);  

    dxgiDevice->Release();
    dxgiAdapter->Release();
    dxgiFactory->Release();

    // The remaining steps that need to be carried out for d3d creation
    // also need to be executed every time the window is resized. So
    // just call the OnResize method here to avoid code duplication.	
    onResize(direct3DData);

    return true;
}

void D3DApplication::calculateFrameStats(WindowData& windowData)
{
    // Code computes the average frames per second, and also the 
    // average time it takes to render one frame.  These stats 
    // are appended to the window caption bar.
    static uint32_t frameCounter = 0;
    static float timeElapsed = 0.0f;

    ++frameCounter;

    // Compute averages over one second period.
    if( (TimerUtils::inGameTime(mTimer) - timeElapsed) >= 1.0f )
    {
        const float fps = static_cast<float> (frameCounter); // fps = frameCnt / 1
        const float mspf = 1000.0f / fps;

        std::wostringstream outs;   
        outs.precision(6);
        outs << mMainWindowCaption << L"    "
            << L"FPS: " << fps << L"    " 
            << L"Frame Time: " << mspf << L" (ms)";

        SetWindowText(windowData.mMainWindow, outs.str().c_str());

        // Reset for next average.
        frameCounter = 0;
        timeElapsed += 1.0f;
    }
}