#include "GPUAcceleratedInterlockingTilesAlgorithmNaive.h"

#include <sstream>
#include <WindowsX.h>

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <DxErrorChecker.h>
#include <GeometryGenerator.h>
#include <MathHelper.h>

#include "Globals.h"

namespace
{
    // This is just used to forward Windows messages from a global window
    // procedure to our member function window procedure because we cannot
    // assign a member function to WNDCLASS::lpfnWndProc.
    GPUAcceleratedInterlockingTilesAlgorithmNaive* gD3DApp = 0;
}

LRESULT CALLBACK
    MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return gD3DApp->msgProc(hwnd, msg, wParam, lParam);
}

GPUAcceleratedInterlockingTilesAlgorithmNaive::GPUAcceleratedInterlockingTilesAlgorithmNaive()
    : mClientWidth(800)
    , mClientHeight(600)
    , m4xMsaaQuality(0)
    , mEnable4xMsaa(false)
    , mWireframeMode(false)
{
    // Get a pointer to the application object so we can forward 
    // Windows messages to the object's window procedure through
    // the global window procedure.
    gD3DApp = this;

    mCamera.mPosition = DirectX::XMFLOAT3(0.0f, 2.0f, -15.0f);

    mLastMousePosition.x = 0;
    mLastMousePosition.y = 0;

    // Terrain world matrix
    const DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
    DirectX::XMStoreFloat4x4(&mWorldMatrix, translation);

    // Texture scale matrix
    const DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 1.0f, 0.0f);
    DirectX::XMStoreFloat4x4(&mTextureScaleMatrix, scale);

    // Directional lights.
    mDirectionalLight[0].mAmbient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    mDirectionalLight[0].mDiffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mDirectionalLight[0].mSpecular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
    mDirectionalLight[0].mDirection = DirectX::XMFLOAT3(0.707f, -0.707f, 0.0f);

    mDirectionalLight[1].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirectionalLight[1].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirectionalLight[1].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirectionalLight[1].mDirection = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    mDirectionalLight[2].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirectionalLight[2].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirectionalLight[2].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirectionalLight[2].mDirection = DirectX::XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);

    // Initialize material
    mTerrainMaterial.mAmbient  = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mTerrainMaterial.mDiffuse  = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mTerrainMaterial.mSpecular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
    mTerrainMaterial.mReflect  = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

GPUAcceleratedInterlockingTilesAlgorithmNaive::~GPUAcceleratedInterlockingTilesAlgorithmNaive()
{
    GlobalsUtils::destroy(gGlobals);
}

bool GPUAcceleratedInterlockingTilesAlgorithmNaive::init(Direct3DData& direct3DData, WindowData& windowData)
{
    if(!initMainWindow(windowData) || !initDirect3D(direct3DData, windowData))
        return false;

    assert(direct3DData.mDevice);
    ConstantBufferUtils::initialize(*direct3DData.mDevice, mGridPSPerFrameBuffer);
    ConstantBufferUtils::initialize(*direct3DData.mDevice, mGridHSPerFrameBuffer);
    ConstantBufferUtils::initialize(*direct3DData.mDevice, mGridDSPerFrameBuffer);
    ConstantBufferUtils::initialize(*direct3DData.mDevice, mGridVSPerObjectBuffer);
        
    assert(direct3DData.mImmediateContext);
    GlobalsUtils::init(
        *direct3DData.mDevice,
        *direct3DData.mImmediateContext,
        gGlobals);

    return true;
}

void GPUAcceleratedInterlockingTilesAlgorithmNaive::updateScene(const float dt)
{
    //
    // Control the camera.
    //
    const float offset = 50.0f;
    if (GetAsyncKeyState('W') & 0x8000)
    {
        CameraUtils::walk(offset * dt, mCamera);
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        CameraUtils::walk(-offset * dt, mCamera);
    }

    if (GetAsyncKeyState('A') & 0x8000)
    {
        CameraUtils::strafe(-offset * dt, mCamera);
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        CameraUtils::strafe(offset * dt, mCamera);
    }

    if (GetAsyncKeyState('T') & 0x8000) 
    {
        mWireframeMode = true;
    }

    if (GetAsyncKeyState('Y') & 0x8000) 
    {
        mWireframeMode = false;
    }
}

int GPUAcceleratedInterlockingTilesAlgorithmNaive::run(Direct3DData& direct3DData, 
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

LRESULT GPUAcceleratedInterlockingTilesAlgorithmNaive::msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

void GPUAcceleratedInterlockingTilesAlgorithmNaive::drawScene(Direct3DData& direct3DData)
{
    direct3DData.mImmediateContext->ClearRenderTargetView(direct3DData.mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Silver));
    direct3DData.mImmediateContext->ClearDepthStencilView(direct3DData.mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    direct3DData.mImmediateContext->RSSetState(mWireframeMode ? gGlobals.mPipelineStates.mWireframeRS : nullptr);

    CameraUtils::updateViewMatrix(mCamera);
       
    drawGrid(direct3DData);

    // Present results
    const HRESULT result = direct3DData.mSwapChain->Present(0, 0);
    DxErrorChecker(result);
}

void GPUAcceleratedInterlockingTilesAlgorithmNaive::onMouseMove(WPARAM btnState, 
                                                                const int32_t x, 
                                                                const int32_t y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        const float dx = DirectX::XMConvertToRadians(0.15f * static_cast<float>(x - mLastMousePosition.x));
        const float dy = DirectX::XMConvertToRadians(0.15f * static_cast<float>(y - mLastMousePosition.y));

        CameraUtils::pitch(dy, mCamera);
        CameraUtils::rotateY(dx, mCamera);
    }

    mLastMousePosition.x = x;
    mLastMousePosition.y = y;
}

void GPUAcceleratedInterlockingTilesAlgorithmNaive::drawGrid(Direct3DData& direct3DData)
{
    //
    // Input Assembler Stage
    //

    // Input Layout 
    ID3D11InputLayout * const inputLayout = gGlobals.mShaders.mTerrainIL;
    direct3DData.mImmediateContext->IASetInputLayout(inputLayout);

    // Primitive Topology
    direct3DData.mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST);

    // Vertex Buffer
    ID3D11Buffer* vertexBuffer = gGlobals.mGeometryBuffers.mBufferInfo->mVertexBuffer;
    uint32_t stride = sizeof(Vertex);
    uint32_t offset = 0;
    direct3DData.mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // Index Buffer
    ID3D11Buffer* indexBuffer = gGlobals.mGeometryBuffers.mBufferInfo->mIndexBuffer;
    direct3DData.mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //
    // Vertex Shader Stage
    //
        
    // Shader
    ID3D11VertexShader * const vertexShader = gGlobals.mShaders.mTerrainVS;
    direct3DData.mImmediateContext->VSSetShader(vertexShader, nullptr, 0);

    // Per Frame Constant Buffer
    const DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mWorldMatrix);
    const DirectX::XMMATRIX textureScale = DirectX::XMLoadFloat4x4(&mTextureScaleMatrix);
    DirectX::XMStoreFloat4x4(&mGridVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));
    DirectX::XMStoreFloat4x4(&mGridVSPerObjectBuffer.mData.mTextureScale, DirectX::XMMatrixTranspose(textureScale));
    ConstantBufferUtils::applyChanges(*direct3DData.mImmediateContext, mGridVSPerObjectBuffer);

    // Set Constant Buffers
    ID3D11Buffer* const vertexShaderBuffers[] = { mGridVSPerObjectBuffer.mBuffer };
    direct3DData.mImmediateContext->VSSetConstantBuffers(0, 1, vertexShaderBuffers);

    //
    // Hull Shader Stage
    //

    // Shader
    ID3D11HullShader * const hullShader = gGlobals.mShaders.mTerrainHS;
    direct3DData.mImmediateContext->HSSetShader(hullShader, nullptr, 0);

    // Per Frame Constant Buffer
    mGridHSPerFrameBuffer.mData.mEyePositionW = mCamera.mPosition;
    ConstantBufferUtils::applyChanges(*direct3DData.mImmediateContext, mGridHSPerFrameBuffer);

    // Set Constant Buffers
    ID3D11Buffer* const hullShaderBuffers = { mGridHSPerFrameBuffer.mBuffer };
    direct3DData.mImmediateContext->HSSetConstantBuffers(0, 1, &hullShaderBuffers);

    //
    // Domain Shader Stage
    //

    // Shader
    ID3D11DomainShader * const domainShader = gGlobals.mShaders.mTerrainDS;
    direct3DData.mImmediateContext->DSSetShader(domainShader, nullptr, 0);

    // Per Frame Constant Buffer
    const DirectX::XMMATRIX viewProjection = CameraUtils::computeViewProjectionMatrix(mCamera);
    const DirectX::XMMATRIX worldInverseTranspose = MathHelper::inverseTranspose(world);
    DirectX::XMStoreFloat4x4(&mGridDSPerFrameBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));
    DirectX::XMStoreFloat4x4(&mGridDSPerFrameBuffer.mData.mViewProjection, DirectX::XMMatrixTranspose(viewProjection));
    const float heightMapTexelSize = 1.0f / 512.0f;
    mGridDSPerFrameBuffer.mData.mHeightMapTexelWidthHeight[0] = heightMapTexelSize;
    mGridDSPerFrameBuffer.mData.mHeightMapTexelWidthHeight[1] = heightMapTexelSize;
    ConstantBufferUtils::applyChanges(*direct3DData.mImmediateContext, mGridDSPerFrameBuffer);

    // Set Constant Buffers
    ID3D11Buffer* const domainShaderBuffers = { mGridDSPerFrameBuffer.mBuffer };
    direct3DData.mImmediateContext->DSSetConstantBuffers(0, 1, &domainShaderBuffers);

    // Resources
    ID3D11ShaderResourceView * const domainShaderResources[] = { gGlobals.mShaderResources.mHeightMapSRV };
    direct3DData.mImmediateContext->DSSetShaderResources(0, 1, domainShaderResources);

    // Sampler state
    direct3DData.mImmediateContext->DSSetSamplers(0, 1, &gGlobals.mPipelineStates.mLinearSS);

    //
    // Pixel Shader Stage
    //

    // Shader
    ID3D11PixelShader* const pixelShader = gGlobals.mShaders.mTerrainPS;
    direct3DData.mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

    // Per Frame Constant Buffer
    memcpy(&mGridPSPerFrameBuffer.mData.mDirectionalLight, &mDirectionalLight, sizeof(mDirectionalLight));
    mGridPSPerFrameBuffer.mData.mEyePositionW = mCamera.mPosition;
    mGridPSPerFrameBuffer.mData.mMaterial = mTerrainMaterial;
    mGridPSPerFrameBuffer.mData.mTexelCellSpaceU = heightMapTexelSize;
    mGridPSPerFrameBuffer.mData.mTexelCellSpaceV = heightMapTexelSize;
    mGridPSPerFrameBuffer.mData.mWorldCellSpace = 0.5f;
    ConstantBufferUtils::applyChanges(*direct3DData.mImmediateContext, mGridPSPerFrameBuffer);

    // Set constant buffers
    ID3D11Buffer * const pixelShaderBuffers[] = { mGridPSPerFrameBuffer.mBuffer };
    direct3DData.mImmediateContext->PSSetConstantBuffers(0, 1, pixelShaderBuffers);
                
    // Resources
    ID3D11ShaderResourceView * const pixelShaderResources[] = 
    { 
        gGlobals.mShaderResources.mTerrainDiffuseMapSRV,
        gGlobals.mShaderResources.mHeightMapSRV
    };
    direct3DData.mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

    // Sampler state
    direct3DData.mImmediateContext->PSSetSamplers(0, 1, &gGlobals.mPipelineStates.mLinearSS);

    //
    // Draw
    // 
    const uint32_t baseVertexLocation = gGlobals.mGeometryBuffers.mBufferInfo->mBaseVertexLocation;
    const uint32_t startIndexLocation = gGlobals.mGeometryBuffers.mBufferInfo->mStartIndexLocation;
    const uint32_t indexCount = gGlobals.mGeometryBuffers.mBufferInfo->mIndexCount;
    direct3DData.mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

bool GPUAcceleratedInterlockingTilesAlgorithmNaive::initMainWindow(WindowData& windowData)
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
        L"GPU Accelerated Interlocking Tiles Algorithm Naive Demo", 
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

bool GPUAcceleratedInterlockingTilesAlgorithmNaive::initDirect3D(Direct3DData& direct3DData, WindowData& windowData)
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

void GPUAcceleratedInterlockingTilesAlgorithmNaive::calculateFrameStats(WindowData& windowData)
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
        outs << L"GPU Accelerated Interlocking Tiles Algorithm Naive Demo" << L"    "
            << L"FPS: " << fps << L"    " 
            << L"Frame Time: " << mspf << L" (ms)";

        SetWindowText(windowData.mMainWindow, outs.str().c_str());

        // Reset for next average.
        frameCounter = 0;
        timeElapsed += 1.0f;
    }
}