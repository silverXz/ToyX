#include "stdafx.h"
#include "Arti3D_Window.h"
#include "Clock.h"
#include "Arti3D_Device.h"

#include <SDL/SDL.h>

Arti3DWindow::Arti3DWindow():m_pWindow(nullptr)
, m_bRunning(false)
{

}

Arti3DWindow::~Arti3DWindow()
{
	if (m_pWindow)
		SDL_DestroyWindow(m_pWindow);
}

Arti3DResult Arti3DWindow::Create(const char *pTitle, int x, int y, int width, int height, uint32_t flag)
{
	m_pWindow = SDL_CreateWindow(pTitle, x, y, width, height, flag);

	m_iWidth = width;
	m_iHeight = height;

	if (m_pWindow)
		return ARTI3D_OK;
	return ARTI3D_INVALID_PARAMETER;
}

void Arti3DWindow::UpdateSurface()
{
	SDL_UpdateWindowSurface(m_pWindow);
}
