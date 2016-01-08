#ifndef _ARTI3D_SURFACE_H_
#define _ARTI3D_SURFACE_H_

#include "Arti3D_Types.h"

struct SDL_Surface;
class Arti3DWindow;

class Arti3DSurface
{


protected:
	friend class Arti3DDevice;
	friend class Arti3DPixelShader;

	// Instance Could Only Be Create By Arti3DDevice
	// @param i_pParent : Pointer To The Arti3DDevice Who Created This Instance.
	Arti3DSurface(Arti3DDevice *i_pParent);

	~Arti3DSurface();

	// Instance Could Only Be Create By Arti3DDevice
	// @param bpp : bits per pixel
	// @param i_iWidth : Width Of The Surface.
	// @param i_iHeight : Height Of The Surface.
	// @param rmask : red channel mask.
	// @param gmask : greed channel mask.
	// @param bmask : blue channel mask.
	// @param amask : alpha channel mask.
	// @return ARTI3D_OK If This Function Succeeds.
	Arti3DResult Create(uint32_t bpp,uint32_t i_iWidth, uint32_t i_iHeight, uint32_t rmask, uint32_t gmask,uint32_t bmask,uint32_t amask);

	// Get Window Associated Surface.
	// @param : pointer to the window that the surface associated with.
	Arti3DResult Create(Arti3DWindow *pWindow);

public:
	Arti3DResult Clear();

	void*	pGetPixelsDataPtr();

	int iGetBitPerPixel();
	int iGetPitch();
	int iGetWidth() ;
	int iGetHeight() ;

	Arti3DFormat	fmtGetFormat() const;

private:
	Arti3DDevice		*m_pParent;

	SDL_Surface			*m_pSurface;

	Arti3DPixelFormat	m_pixelFormat;
};


#endif