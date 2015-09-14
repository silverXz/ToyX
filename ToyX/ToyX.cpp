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



#include "Arti3D_Device.h"
#include "Arti3D_VertexLayout.h"
#include "Arti3D_VertexBuffer.h"
#include "Arti3D_IndexBuffer.h"

using namespace a3d;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

inline uint32_t ToRGB(int r, int g, int b)
{
	return ((255 << 24) | (r << 16) | (g << 8) | b);
}

//The window we'll be rendering to
SDL_Window* g_Window = NULL;


static bool g_Running = true;


void PressAnyKeyToContinue()
{
	std::cout << "Press Any Key To Continue:";
	getchar();
}



static void HandleKeyEvent(const SDL_Event &event)
{
	switch (event.key.keysym.sym) 
	{
	case SDLK_ESCAPE:
		g_Running = false;
		break;
	default:
		break;
	}
}

static void HandleEvent(const SDL_Event &event)
{
	switch (event.type) 
	{
	case SDL_QUIT:
		g_Running = false;
		break;
	case SDL_KEYDOWN:
		HandleKeyEvent(event);
		break;
	default:
		break;
	}
}

Arti3DResult CreateAndInitializeDevice(Arti3DDevice **io_pArti3DDev,Arti3DDeviceParameter *pA3DDeviceParameters)
{
	if (!io_pArti3DDev)
		return ARTI3D_INVALID_PARAMETER;

	*io_pArti3DDev = new Arti3DDevice;
	
	if (!*io_pArti3DDev)
		return ARTI3D_OUT_OF_MEMORY;

	(*io_pArti3DDev)->InitializeDevice(*pA3DDeviceParameters);

	SDL_Surface *cb = SDL_GetWindowSurface(g_Window);
	if (!cb)
		return ARTI3D_OUT_OF_MEMORY;

	SDL_Surface *zb = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0, 0, 0, 0);
	if (!zb)
		return ARTI3D_OUT_OF_MEMORY;

	SDL_Surface *tb = SDL_CreateRGBSurface(0, 1024, 1024, 32, 0, 0, 0, 0);
	if (!tb)
		return ARTI3D_OUT_OF_MEMORY;

	RenderTarget rt;
	rt.back_buffer = cb;
	rt.z_buffer = zb;

	(*io_pArti3DDev)->SetRenderTarget(rt);
	
	(*io_pArti3DDev)->SetMatrix(TOY_MATRIX_VIEW, lookAt(vec3(4.0f, 4.0f, 4.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	(*io_pArti3DDev)->SetMatrix(TOY_MATRIX_PROJECTION, perspective(90.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 15.0f));
	(*io_pArti3DDev)->SetViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	(*io_pArti3DDev)->SetVertexShader(NewCubeVS);
	(*io_pArti3DDev)->SetPixelShader(NewCubeFS);

	// Load Obj.

	const float len = 2.0f;

	Arti3DVertexLayout *pVertexLayout = nullptr;
	Arti3DVertexAttributeFormat vaf[] = { ARTI3D_VAF_VECTOR4, ARTI3D_VAF_VECTOR4 };
	(*io_pArti3DDev)->CreateVertexLayout(&pVertexLayout, 2, vaf);
	(*io_pArti3DDev)->SetVertexLayout(pVertexLayout);

	Arti3DVertexBuffer *pVertexBuffer = nullptr;

	uint32_t iFloat = pVertexLayout->iGetFloats();
	uint32_t iStride = iFloat * sizeof(float);
	const uint32_t iVertex = 8;
	(*io_pArti3DDev)->CreateVertexBuffer(&pVertexBuffer, iVertex * iStride);

	// Upload Cube Data To VertexBuffer
	std::vector<std::vector<float>> xv{
		{ -len, len, len, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f },	//0
		{ len, len, len, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f },	//1
		{ len, len, -len, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f },	//2
		{ -len, len, -len, 1.0f,	0.0f, 1.0f, 1.0f, 1.0f },	//3
		{ -len, -len, len, 1.0f,	1.0f, 0.0f, 1.0f, 1.0f },	//4
		{ len, -len, len, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f },	//5
		{ len, -len, -len, 1.0f,	1.0f, 1.0f, 1.0f, 0.0f },	//6
		{ -len, -len, -len, 1.0f,	0.0f, 0.0f, 0.0f, 1.0f }	//7
	};

	for (int i = 0; i < iVertex; ++i)
	{
		void *pDest = nullptr;
		pVertexBuffer->GetPointer(i * iStride, &pDest);
		memcpy(pDest, &xv[i][0], iStride);
	}

	(*io_pArti3DDev)->SetVertexBuffer(pVertexBuffer);

	Arti3DIndexBuffer *pIndexBuffer = nullptr;
	(*io_pArti3DDev)->CreateIndexBuffer(&pIndexBuffer, 36 * sizeof(uint32_t), ARTI3D_INDEX32);
	uint32_t xid[] = { 0, 1, 2, 0, 2, 3, 0, 4, 5, 0, 5, 1, 1, 5, 6, 1, 6, 2, 4, 7, 6, 4, 6, 5, 0, 3, 7, 0, 7, 4, 3, 2, 6, 3, 6, 7 };
	void *pDest = nullptr;
	pIndexBuffer->GetPointer(0, &pDest);
	memcpy(pDest, xid, sizeof(xid));

	(*io_pArti3DDev)->SetIndexBuffer(pIndexBuffer);

	(*io_pArti3DDev)->InitializeWorkThreads();

	return ARTI3D_OK;
}

void DestroyDevice(Arti3DDevice **io_pArti3DDev)
{
	if (!io_pArti3DDev)
		return;
	if (*io_pArti3DDev)
		delete *io_pArti3DDev;
	*io_pArti3DDev = nullptr;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf_s(stderr, "SDL_Init Failed!\n");
		return 1;
	}

	g_Window = SDL_CreateWindow("ToyX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

	if (!g_Window)
	{
		SDL_Quit();
		fprintf_s(stderr, "SDL_SetVideoMode Failed!\n");
		return 1;
	}

	Arti3DDeviceParameter a3dDeviceParameter;
	a3dDeviceParameter.bMultiThread = true;
	a3dDeviceParameter.iWidth = WINDOW_WIDTH;
	a3dDeviceParameter.iHeight = WINDOW_HEIGHT;

	Arti3DDevice *pArti3DDev = nullptr;
	CreateAndInitializeDevice(&pArti3DDev,&a3dDeviceParameter);


	auto past = 0.0;

	auto last = iv::Clock::GetCurrentTimeMS();

	auto nFrame = 0;

	auto rotAngle = 0.0f;
	const auto rotSpeed = a3d::PI / 20.0f;

	while (g_Running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			HandleEvent(event);
		double curTime = iv::Clock::GetCurrentTimeMS();

		past += curTime - last;
		auto dt = (curTime - last) * 0.001;
		rotAngle += rotSpeed * dt;
		if (rotAngle > a3d::TWOPI)
			rotAngle -= a3d::TWOPI;

		pArti3DDev->SetMatrix(TOY_MATRIX_MODEL, a3d::rotate(rotAngle, a3d::vec3(0.0f, 1.0f, 0.0f)));

		last = curTime;

		if (past >= 1000.0)
		{
			std::cout << "FPS:" << nFrame << std::endl;
			nFrame = 0;
			past = 0.0;
		}
		else
			++nFrame;

		pArti3DDev->Begin();
		pArti3DDev->ClearColorBuffer(ToyColor(0.0f, 0.0f, 0.0f));
		pArti3DDev->ClearDepthBuffer();
		//pArti3DDev->DrawMesh_TileBase();
		pArti3DDev->DrawMesh_MT();
		pArti3DDev->End();

		SDL_UpdateWindowSurface(g_Window);
	}

	SDL_Quit();
	DestroyDevice(&pArti3DDev);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	_CrtDumpMemoryLeaks();

	return 0;

}

