#pragma once

#include <D3DX11.h>
#include <dxerr.h>

namespace DebugUtils
{
#if defined(DEBUG) | defined(_DEBUG)
    __forceinline void ErrorChecker(const HRESULT result)                                    
    {                                          
        if (result < 0)                                         
            DXTrace(__FILE__, __LINE__, result, 0, true); 			                                                      
    }

#else
    __forceinline void ErrorChecker(const HRESULT result) 
    {

    }
#endif 
}