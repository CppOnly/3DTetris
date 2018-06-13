#include "stdafx.h"
#include "Tetris3D.h"

int WINAPI WinMain(HINSTANCE _hInstance, HINSTANCE _prevInstance, PSTR _cmdLine, int _showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	// Test
	try {
		Tetris3D theApp(_hInstance);
		if (!theApp.Initialize()) {
			return 0;
		}

		return theApp.RunWindow();
	}
	catch (DxException& e) {
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}