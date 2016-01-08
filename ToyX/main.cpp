// ToyX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdint.h>
#include <windows.h>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#include "Arti3DApp.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Arti3DApp a3dApp;
		
	if (ARTI3D_OK != a3dApp.Init())
		return -1;

	a3dApp.SetupScene2();

	a3dApp.Run();

	_CrtDumpMemoryLeaks();

	return 0;
}

