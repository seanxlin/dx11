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

        ShadersUtils::initAll(*Globals::gDirect3DData.mDevice, Globals::gShaders);

        ShaderResourcesUtils::initAll(
            *Globals::gDirect3DData.mDevice, 
            *Globals::gDirect3DData.mImmediateContext, 
            Globals::gShaderResources);
        
        PipelineStatesUtils::initAll(*Globals::gDirect3DData.mDevice, Globals::gPipelineStates);

        GeometryBuffersUtils::initAll(*Globals::gDirect3DData.mDevice, Globals::gGeometryBuffers);
    
        return success;
    }

    void destroy()
    {
        ShadersUtils::destroyAll(Globals::gShaders);

        ShaderResourcesUtils::destroyAll(Globals::gShaderResources);

        PipelineStatesUtils::destroyAll(Globals::gPipelineStates);

        GeometryBuffersUtils::destroyAll(Globals::gGeometryBuffers);

        Direct3DDataUtils::destroy(Globals::gDirect3DData);
    }
}