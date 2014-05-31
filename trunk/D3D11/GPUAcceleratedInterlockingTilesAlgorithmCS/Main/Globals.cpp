#include "Globals.h"

HINSTANCE Globals::gAppInstance;
WindowData Globals::gWindowData;
Direct3DData Globals::gDirect3DData;
Shaders Globals::gShaders;
ShaderResources Globals::gShaderResources;
PipelineStates Globals::gPipelineStates;
GeometryBuffers Globals::gGeometryBuffers;
Timer Globals::gTimer;
Camera Globals::gCamera;
MouseProperties Globals::gMouseProperties;
WindowState Globals::gWindowState;

namespace GlobalsUtils
{
    bool init()
    {
        const bool success = WindowDataUtils::init(Globals::gWindowData) && 
                             Direct3DDataUtils::init(Globals::gDirect3DData, Globals::gWindowData);

        if (!success)
        {
            return success;
        }

        ShadersUtils::init(*Globals::gDirect3DData.mDevice, Globals::gShaders);

        ShaderResourcesUtils::init(
            *Globals::gDirect3DData.mDevice, 
            *Globals::gDirect3DData.mImmediateContext, 
            Globals::gShaderResources);
        
        PipelineStatesUtils::init(*Globals::gDirect3DData.mDevice, Globals::gPipelineStates);

        GeometryBuffersUtils::init(*Globals::gDirect3DData.mDevice, Globals::gGeometryBuffers);
    
        return success;
    }

    void destroy()
    {
        ShadersUtils::destroy(Globals::gShaders);

        ShaderResourcesUtils::destroy(Globals::gShaderResources);

        PipelineStatesUtils::destroy(Globals::gPipelineStates);

        GeometryBuffersUtils::destroy(Globals::gGeometryBuffers);

        Direct3DDataUtils::destroy(Globals::gDirect3DData);
    }
}