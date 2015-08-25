// ToyX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <SDL/SDL.h>
#include <memory.h>
#include <stdint.h>
#include <windows.h>

#include "Clock.h"
#include <xmmintrin.h>	//SSE
#include <smmintrin.h>	//SSE4

#include "Shader.h"



#include "ToyRender.h"


using namespace toy;

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

int _tmain(int argc, _TCHAR* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf_s(stderr, "SDL_Init Failed!\n");
		return 1;
	}

// 	SSE_ALIGN float v1[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
// 	double t0 = iv::Clock::GetCurrentTimeMS();
// 	for (int i = 0; i < 1000000000; ++i)
// 	{
// 		int res = _mm_extract_ps(_mm_sqrt_ss(_mm_dp_ps(*(__m128*)v1, *(__m128*)v1, 0xFF)), 0);
// 		float result = *(float*)(&res);
// 	}
// 	double t1 = iv::Clock::GetCurrentTimeMS();
// 
// 	printf("SSE : cost : %f,\n", t1 - t0); 
// 
// 	t0 = iv::Clock::GetCurrentTimeMS();
// 	for (int i = 0; i < 1000000000; ++i)
// 	{
// 		float s = v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] + v1[3] * v1[3];
// 		float res = sqrt(s);
// 	}
// 	t1 = iv::Clock::GetCurrentTimeMS();
// 
// 	printf("No SSE cost: %f\n", t1 - t0);

	g_Window = SDL_CreateWindow("ToyX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

	if (!g_Window)
	{
		SDL_Quit();
		fprintf_s(stderr, "SDL_SetVideoMode Failed!\n");
		return 1;
	}

	//获取系统高速缓存行的信息
// 	SYSTEM_LOGICAL_PROCESSOR_INFORMATION slpi;
// 	DWORD len;
// 	GetLogicalProcessorInformation(&slpi, &len);
// 
//  	WORD cacheSize = slpi.Cache.LineSize;
// 
// 	printf("CacheLineSize: %d", cacheSize);

	ToyRender toyRender;

 	SDL_Surface *cb = SDL_GetWindowSurface(g_Window);

	if (!cb)
	{
		std::cerr << "Failed to create color buffer!\n";
		PressAnyKeyToContinue();
		return -1;
	}

	SDL_Surface *zb = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0, 0, 0, 0);

	if (!zb)
	{
		std::cerr << "Failed to create z-buffer!\n";
		PressAnyKeyToContinue();
		return -1;
	}

	SDL_Surface *tb = SDL_CreateRGBSurface(0, 1024, 1024, 32, 0, 0, 0, 0);
	if (!tb)
	{
		std::cerr << "Failed to create texture buffer!\n";
		PressAnyKeyToContinue();
		return -1;
	}

	toyRender.SetRenderTarget(cb, zb,tb);


	auto past = 0.0;

	auto last = iv::Clock::GetCurrentTimeMS();

	auto nFrame = 0;

	toyRender.SetMatrix(TOY_MATRIX_VIEW, lookAt(vec3(4.0f, 4.0f,4.0f), vec3(0.0f,0.0f,0.0f), vec3(0.0f, 1.0f, 0.0f)));
	toyRender.SetMatrix(TOY_MATRIX_PROJECTION, perspective(90.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 15.0f));
	toyRender.SetViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	toyRender.SetVertexShader(CubeVS);
	toyRender.SetFragmentShader(CubeFS);
	toyRender.LoadCube();

	auto rotAngle = 0.0f;
	const auto rotSpeed = toy::PI / 20.0f;

	while (g_Running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			HandleEvent(event);

		double curTime = iv::Clock::GetCurrentTimeMS();
		
		past += curTime - last;

		float dt = (curTime - last) * 0.001f;
		rotAngle += rotSpeed * dt;
		if (rotAngle > toy::TWOPI)
			rotAngle -= toy::TWOPI;

		//toyRender.SetMatrix(TOY_MATRIX_VIEW, lookAt(vec3(4.0f + xMove, 4.0f, 4.0f + zMove), vec3(0.0f + xMove, 0.0f, 0.0f + zMove), vec3(0.0f, 1.0f, 0.0f)));
		toyRender.SetMatrix(TOY_MATRIX_MODEL, toy::rotate(rotAngle, toy::vec3(0.0f, 1.0f, 0.0f)));

		last = curTime;

		if (past >= 1000.0)
		{
			std::cout << "FPS:" << nFrame << std::endl;
			nFrame = 0;
			past = 0.0;
		}
		else
			++nFrame;

		toyRender.Begin();


		toyRender.ClearColorBuffer(ToyColor(0.0f,0.0f,0.0f));
		toyRender.ClearDepthBuffer();

		toyRender.DrawMesh();
		
		toyRender.End();

		SDL_UpdateWindowSurface(g_Window);



	}

	SDL_Quit();

	return 0;

}

