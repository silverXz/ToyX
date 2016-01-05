// ToyX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <SDL/SDL.h>
#include <memory.h>
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



#include "Clock.h"
#include <xmmintrin.h>	//SSE
#include <smmintrin.h>	//SSE4

#include "Shader.h"

#include "Arti3DApp.h"

#include "Arti3D_Device.h"
#include "Arti3D_VertexLayout.h"
#include "Arti3D_VertexBuffer.h"
#include "Arti3D_IndexBuffer.h"

using namespace a3d;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int _tmain(int argc, _TCHAR* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf_s(stderr, "SDL_Init Failed!\n");
		return -1;
	}

	Arti3DApp *pApp = new Arti3DApp;

	if (!pApp)
		return -1;

	Arti3DWindow *pWindow = nullptr;
	
	Arti3DResult a3dResult = pApp->CreateArti3DWindow(&pWindow,"ToyX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (a3dResult != ARTI3D_OK)
		return -1;

	Arti3DDeviceParameter a3dDeviceParameter;
	a3dDeviceParameter.bMultiThread = true;
	a3dDeviceParameter.iWidth = WINDOW_WIDTH;
	a3dDeviceParameter.iHeight = WINDOW_HEIGHT;

	Arti3DDevice *pDevice = nullptr;
	a3dResult = pApp->CreateAndInitializeDevice(&pDevice, &a3dDeviceParameter);

	pApp->Run();

	delete pApp;

	std::this_thread::sleep_for(std::chrono::seconds(1));

	_CrtDumpMemoryLeaks();

	return 0;

}

