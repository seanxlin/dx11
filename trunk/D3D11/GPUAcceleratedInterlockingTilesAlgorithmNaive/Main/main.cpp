#include "GPUAcceleratedInterlockingTilesAlgorithmNaive.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    gWindowData.mAppInstance = hInstance;

    GPUAcceleratedInterlockingTilesAlgorithmNaive gpuAcceleratedInterlockingTilesAlgorithmNaive; 

    if (!gpuAcceleratedInterlockingTilesAlgorithmNaive.init(gDirect3DData, gWindowData))
        return 0;

    return gpuAcceleratedInterlockingTilesAlgorithmNaive.run(gDirect3DData, gWindowState, gWindowData);
}