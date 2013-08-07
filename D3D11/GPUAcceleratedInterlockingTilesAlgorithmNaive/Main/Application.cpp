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
    static TerrainScene gTerrainScene;

    void calculateFrameStats()
    {
        // Computes the average frames per second, and also the 
        // average time it takes to render one frame. These stats 
        // are appended to the window caption bar.
        static uint32_t sFrameCounter = 0;
        static float sTimeElapsed = 0.0f;

        ++sFrameCounter;

        // Compute averages over one second period.
        if( (TimerUtils::inGameTime(Globals::gTimer) - sTimeElapsed) >= 1.0f) {
            // fps = frameCnt / 1
            const float framesPerSecond = static_cast<float> (sFrameCounter); 
            const float millisecondsPerFrame = 1000.0f / framesPerSecond;

            std::wostringstream outs;   
            outs.precision(6);
            outs << L"GPU Accelerated Interlocking Tiles Algorithm Naive Demo" << L"    "
                << L"FPS: " << framesPerSecond << L"    " 
                << L"Frame Time: " << millisecondsPerFrame << L" (ms)";

            SetWindowText(Globals::gWindowData.mMainWindow, outs.str().c_str());

            // Reset for next average.
            sFrameCounter = 0;
            sTimeElapsed += 1.0f;
        }
    }

    void updateScene(const float dt)
    {
        assert(dt > 0.0f);

        //
        // Control the camera.
        //
        const float factor = 50.0f;
        const float distance = factor * dt;
        if (GetAsyncKeyState('W') & 0x8000) {
            CameraUtils::walk(distance, Globals::gCamera);
        }

        if (GetAsyncKeyState('S') & 0x8000) {
            CameraUtils::walk(-1.0f * distance, Globals::gCamera);
        }

        if (GetAsyncKeyState('A') & 0x8000) {
            CameraUtils::strafe(-1.0f * distance, Globals::gCamera);
        }

        if (GetAsyncKeyState('D') & 0x8000) {
            CameraUtils::strafe(distance, Globals::gCamera);
        }

        if (GetAsyncKeyState('T') & 0x8000) {
            Globals::gWindowData.mWireframeMode = true;
        } else if (GetAsyncKeyState('Y') & 0x8000) {
            Globals::gWindowData.mWireframeMode = false;
        }
    }

    void drawScene()
    {
        assert(Globals::gDirect3DData.mImmediateContext);

        Globals::gDirect3DData.mImmediateContext->ClearRenderTargetView(Globals::gDirect3DData.mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Silver));
        Globals::gDirect3DData.mImmediateContext->ClearDepthStencilView(Globals::gDirect3DData.mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        Globals::gDirect3DData.mImmediateContext->RSSetState(Globals::gWindowData.mWireframeMode ? Globals::gPipelineStates.mWireframeRS : nullptr);

        CameraUtils::updateViewMatrix(Globals::gCamera);

        TerrainSceneUtils::draw(gTerrainScene);

        // Present results
        const HRESULT result = Globals::gDirect3DData.mSwapChain->Present(0, 0);
        DxErrorChecker(result);
    }

    void destroy() {
        GlobalsUtils::destroy();
    }
}

namespace Application
{
    bool initApplication()
    {
        const bool success = GlobalsUtils::init();
        if (!success) {
            return success;
        }

        TerrainSceneUtils::init(gTerrainScene);

        Globals::gCamera.mPosition = DirectX::XMFLOAT3(0.0f, 100.0f, 0.0f);
        Globals::gCamera.mLook = DirectX::XMFLOAT3(0.0f, -1.0f, 1.0f);

        return success;
    }

    int executeApplication()
    {
        MSG msg = {0};

        TimerUtils::reset(Globals::gTimer);

        while (msg.message != WM_QUIT) {
            // If there are Window messages then process them.
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {	
                // Otherwise, do animation/game stuff.
                TimerUtils::tick(Globals::gTimer);

                if (!Globals::gWindowState.mIsPaused) {
                    calculateFrameStats();
                    updateScene(static_cast<float> (Globals::gTimer.mDeltaTime));	
                    drawScene();
                } else {
                    Sleep(100);
                }
            }
        }

        destroy();

        return static_cast<int> (msg.wParam);
    }
}