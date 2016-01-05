#include "stdafx.h"
#include "Arti3D_RenderTarget.h"
#include "Arti3D_Surface.h"

Arti3DRenderTarget::Arti3DRenderTarget(Arti3DDevice *i_pParent) :
m_pParent(i_pParent)
{

}

Arti3DRenderTarget::~Arti3DRenderTarget()
{
}

Arti3DDevice* Arti3DRenderTarget::pGetDevice()
{
	return m_pParent;
}

Arti3DResult Arti3DRenderTarget::ClearColorBuffer(uint32_t color)
{
	return ARTI3D_OK;
}

Arti3DResult Arti3DRenderTarget::ClearDepthBuffer()
{
	return ARTI3D_OK;
}

void Arti3DRenderTarget::SetBackBuffer(Arti3DSurface *pSurface)
{
	m_pBackbuffer = pSurface;
}

void Arti3DRenderTarget::SetZBuffer(Arti3DSurface *pSurface)
{
	m_pZBuffer = pSurface;
}

Arti3DSurface* Arti3DRenderTarget::pGetBackBuffer() const
{
	return m_pBackbuffer;
}

Arti3DSurface* Arti3DRenderTarget::pGetZBuffer() const
{
	return m_pZBuffer;
}
