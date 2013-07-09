#include "Application.h"

#include <DirectXColors.h>
#include <sstream>
#include <windowsx.h>

#include <Camera.h>
#include <DxErrorChecker.h>

#include "Globals.h"
#include "Scene.h"

namespace
{
    void calculateFrameStats()
    {
        // Code computes the average frames per second, and also the 
        // average time it takes to render one frame.  These stats 
        // are appended to the window caption bar.
        static uint32_t frameCounter = 0;
        static float timeElapsed = 0.0f;

        ++frameCounter;

        // Compute averages over one second period.
        if( (TimerUtils::inGameTime(Globals::gTimer) - timeElapsed) >= 1.0f )
        {
            const float fps = static_cast<float> (frameCounter); // fps = frameCnt / 1
            const float mspf = 1000.0f / fps;

            std::wostringstream outs;   
            outs.precision(6);
            outs << L"GPU Accelerated Interlocking Tiles Algorithm Naive Demo" << L"    "
                << L"FPS: " << fps << L"    " 
                << L"Frame Time: " << mspf << L" (ms)";

            SetWindowText(Globals::gWindowData.mMainWindow, outs.str().c_str());

            // Reset for next average.
            frameCounter = 0;
            timeElapsed += 1.0f;
        }
    }

    static TerrainScene gTerrainScene;
}

namespace Application
{
    bool init()
    {
        const bool success = GlobalsUtils::init();
        if (!success) 
        {
            return success;
        }

        TerrainSceneUtils::init(gTerrainScene);

        Globals::gCamera.mPosition = DirectX::XMFLOAT3(0.0f, 100.0f, 0.0f);
        Globals::gCamera.mLook = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);

        return success;
    }

    void destroy()
    {
        GlobalsUtils::destroy();
    }

    void updateScene(const float dt)
    {
        //
        // Control the camera.
        //
        const float offset = 50.0f;
        if (GetAsyncKeyState('W') & 0x8000)
        {
            CameraUtils::walk(offset * dt, Globals::gCamera);
        }

        if (GetAsyncKeyState('S') & 0x8000)
        {
            CameraUtils::walk(-offset * dt, Globals::gCamera);
        }

        if (GetAsyncKeyState('A') & 0x8000)
        {
            CameraUtils::strafe(-offset * dt, Globals::gCamera);
        }

        if (GetAsyncKeyState('D') & 0x8000)
        {
            CameraUtils::strafe(offset * dt, Globals::gCamera);
        }

        if (GetAsyncKeyState('T') & 0x8000) 
        {
            Globals::gWindowData.mWireframeMode = true;
        }

        if (GetAsyncKeyState('Y') & 0x8000) 
        {
            Globals::gWindowData.mWireframeMode = false;
        }
    }

    int run()
    {
        MSG msg = {0};

        TimerUtils::reset(Globals::gTimer);

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
                TimerUtils::tick(Globals::gTimer);

                if (!Globals::gWindowState.mIsPaused)
                {
                    calculateFrameStats();
                    updateScene(static_cast<float> (Globals::gTimer.mDeltaTime));	
                    drawScene();
                }
                else
                {
                    Sleep(100);
                }
            }
        }

        destroy();
        return static_cast<int> (msg.wParam);
    }

    void drawScene()
    {
        assert(Globals::gDirect3DData.mImmediateContext);

        Globals::gDirect3DData.mImmediateContext->ClearRenderTargetView(Globals::gDirect3DData.mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Black));
        Globals::gDirect3DData.mImmediateContext->ClearDepthStencilView(Globals::gDirect3DData.mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        Globals::gDirect3DData.mImmediateContext->RSSetState(Globals::gWindowData.mWireframeMode ? Globals::gPipelineStates.mWireframeRS : nullptr);

        CameraUtils::updateViewMatrix(Globals::gCamera);

        TerrainSceneUtils::draw(gTerrainScene);

        // Present results
        const HRESULT result = Globals::gDirect3DData.mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }
}