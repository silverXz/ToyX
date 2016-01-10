#include "stdafx.h"
#include "Arti3D_Surface.h"
#include "Arti3D_Device.h"
#include "Arti3D_Window.h"

#include <SDL/SDL.h>

Arti3DSurface::Arti3DSurface(Arti3DDevice *i_pParent) : m_pParent(i_pParent)
{
	memset(&m_pixelFormat, 0, sizeof(Arti3DPixelFormat));
}

Arti3DSurface::~Arti3DSurface()
{
	if (m_pSurface)
		SDL_FreeSurface(m_pSurface);
}

Arti3DResult Arti3DSurface::Create(uint32_t bpp, uint32_t i_iWidth, uint32_t i_iHeight, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask)
{
	m_pSurface = SDL_CreateRGBSurface(0, i_iWidth, i_iHeight, bpp, rmask, gmask, bmask, amask);

	if (!m_pSurface)
		return ARTI3D_INVALID_FORMAT;

	m_pixelFormat.BitsPerPixel = bpp;
	m_pixelFormat.BytesPerPixel = bpp / 8;
	
	m_pixelFormat.RMask = rmask;
	m_pixelFormat.GMask = gmask;
	m_pixelFormat.BMask = bmask;
	m_pixelFormat.AMask = amask;

	// Uncompleted version!!!!!!!
	m_pixelFormat.Rshift = (rmask == 0xFF000000) ? 24 : (rmask == 0x000000FF00 ? 8 : 0);
	m_pixelFormat.Gshift = (gmask == 0x00FF0000) ? 16 : 8;
	m_pixelFormat.Bshift = (bmask == 0x0000FF00) ? 8 : 16;
	m_pixelFormat.Ashift = (amask == 0x000000FF) ? 0 : 24;
	
	return ARTI3D_OK;
}

Arti3DResult Arti3DSurface::Create(Arti3DWindow *pWindow)
{
	if (!pWindow)
		return ARTI3D_INVALID_PARAMETER;
	m_pSurface = SDL_GetWindowSurface(pWindow->m_pWindow);
	
	if (!m_pSurface)
		return ARTI3D_INVALID_PARAMETER;
	
	m_pixelFormat.BitsPerPixel = 32;
	m_pixelFormat.BytesPerPixel = 4;

	m_pixelFormat.RMask = 0xFF000000;
	m_pixelFormat.GMask = 0x00FF0000;
	m_pixelFormat.BMask = 0x0000FF00;
	m_pixelFormat.AMask = 0x000000FF;

	// Uncompleted version!!!!!!!
	m_pixelFormat.Rshift = 24;
	m_pixelFormat.Gshift = 16;
	m_pixelFormat.Bshift = 8;
	m_pixelFormat.Ashift = 0;

	return ARTI3D_OK;
}

void Arti3DSurface::Lock()
{
	SDL_LockSurface(m_pSurface);
}

void Arti3DSurface::UnLock()
{
	SDL_UnlockSurface(m_pSurface);
}

void* Arti3DSurface::pGetPixelsDataPtr()
{
	return m_pSurface->pixels;
}

int Arti3DSurface::iGetBitPerPixel()
{
	return m_pixelFormat.BitsPerPixel;
}

int Arti3DSurface::iGetPitch()
{
	return m_pSurface->pitch;
}

int Arti3DSurface::iGetWidth()
{
	return m_pSurface->w;
}

int Arti3DSurface::iGetHeight() 
{
	return m_pSurface->h;
}
