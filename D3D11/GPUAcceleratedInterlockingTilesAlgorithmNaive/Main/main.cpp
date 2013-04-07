#include "Globals.h"
#include "GPUAcceleratedInterlockingTilesAlgorithmNaive.h"

#include <crtdbg.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    
    Globals::gAppInstance = hInstance;
    
    GPUAcceleratedInterlockingTilesAlgorithmNaive gpuAcceleratedInterlockingTilesAlgorithmNaive; 

    if (!gpuAcceleratedInterlockingTilesAlgorithmNaive.init())
    {
        return 0;
    }

    return run();
}