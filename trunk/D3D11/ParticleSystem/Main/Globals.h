//////////////////////////////////////////////////////////////////////////
//
// Global useful data
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Camera.h>
#include <Timer.h>

#include <Managers/GeometryBuffersManager.h>
#include <Managers/PipelineStatesManager.h>
#include <Managers/ShaderResourcesManager.h>
#include <Managers/ShadersManager.h>

#include "D3DData.h"
#include "WindowManager.h"

struct Globals
{
    static HINSTANCE gAppInstance;
    static WindowData gWindowData;
    static Direct3DData gDirect3DData;
    static Shaders gShaders;
    static ShaderResources gShaderResources;
    static PipelineStates gPipelineStates;
    static GeometryBuffers gGeometryBuffers;
    static Timer gTimer;
    static Camera gCamera;
    static MouseProperties gMouseProperties;
    static WindowState gWindowState;
};

namespace GlobalsUtils
{
    bool init();
    void destroy();
}