#include "stdafx.h"
#include "Arti3D_Surface.h"
#include "Arti3D_Device.h"
#include "Arti3D_Window.h"

#include <SDL/SDL.h>


Arti3DSurface::Arti3DSurface(Arti3DDevice *i_pParent) : m_pParent(i_pParent)
,m_iWidth(0)
, m_iHeight(0)
, m_fmtFormat(ARTI3D_FORMAT_INVLAID)

{

}

Arti3DSurface::~Arti3DSurface()
{
	if (m_pSurface)
		SDL_FreeSurface(m_pSurface);
}

Arti3DResult Arti3DSurface::Create(uint32_t bpp, uint32_t i_iWidth, uint32_t i_iHeight, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask)
{
	m_pSurface = SDL_CreateRGBSurface(0, i_iWidth, i_iHeight, bpp, rmask, gmask, bmask, amask);
	m_iWidth = i_iWidth;
	m_iHeight = i_iHeight;
	m_fmtFormat = m_pParent->MasksToPixelsFormatEnum(bpp, rmask, gmask, bmask, amask);

	if (!m_pSurface)
		return ARTI3D_INVALID_FORMAT;
	return ARTI3D_OK;
}

Arti3DResult Arti3DSurface::Create(Arti3DWindow *pWindow)
{
	if (!pWindow)
		return ARTI3D_INVALID_PARAMETER;
	m_pSurface = SDL_GetWindowSurface(pWindow->m_pWindow);
	
	if (!m_pSurface)
		return ARTI3D_INVALID_PARAMETER;
	
	m_iWidth = m_pSurface->w;
	m_iHeight = m_pSurface->h;
	m_fmtFormat = ARTI3D_FORMAT_RGBA8888;
	
	return ARTI3D_OK;
}

void* Arti3DSurface::pGetPixelsDataPtr()
{
	return m_pSurface->pixels;
}

int Arti3DSurface::iGetWidth() 
{
	return m_iWidth;
}

int Arti3DSurface::iGetHeight() 
{
	return m_iHeight;
}
