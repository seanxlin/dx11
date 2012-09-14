#include "InitD3DApp.h"
 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	Framework::InitD3DApp initD3DApp(hInstance);
	
	if (!initD3DApp.init())
		return 0;
	
	return initD3DApp.run();
}