// ToyX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <SDL/SDL.h>
#include <memory.h>
#include <stdint.h>

#include "ToyRender.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define RGB(r,g,b) ((255 << 24) | (r << 16) | (g << 8) | b )

//The window we'll be rendering to
SDL_Window* g_Window = NULL;

//The surface contained by the window
SDL_Surface* g_ScreenSurface = NULL;

static bool g_Running = true;

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

	g_Window = SDL_CreateWindow("ToyX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

	if (!g_Window)
	{
		SDL_Quit();
		fprintf_s(stderr, "SDL_SetVideoMode Failed!\n");
		return 1;
	}

	g_ScreenSurface = SDL_GetWindowSurface(g_Window);

	ToyRender toyRender;

	toyRender.InitBuffers(WINDOW_WIDTH, WINDOW_HEIGHT, (uint32_t*)g_ScreenSurface->pixels, true);

	while (g_Running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			HandleEvent(event);

		SDL_LockSurface(g_ScreenSurface);

		toyRender.ClearColorBuffer(ToyColor(1.0f,0.0f,0.0f));

		SDL_UnlockSurface(g_ScreenSurface);

		SDL_UpdateWindowSurface(g_Window);
	}

	SDL_Quit();

	return 0;

}

