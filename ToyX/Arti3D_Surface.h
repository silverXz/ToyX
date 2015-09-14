#ifndef _ARTI3D_SURFACE_H_
#define _ARTI3D_SURFACE_H_

#include "Arti3D_Types.h"

class SDL_Surface;


class Arti3DSurface
{
protected:
	friend class Arti3DDevice;


	// Instance Could Only Be Create By Arti3DDevice
	// @param i_pParent : Pointer To The Arti3DDevice Who Created This Instance.
	Arti3DSurface(Arti3DDevice *i_pParent);

	~Arti3DSurface();

	// Instance Could Only Be Create By Arti3DDevice
	// @param i_iWidth : Width Of The Surface.
	// @param i_iHeight : Height Of The Surface.
	// @param i_fmtFormat : Pixel's Format.
	// @return ARTI3D_OK If This Function Succeeds.
	Arti3DResult Create(uint32_t i_iWidth, uint32_t i_iHeight, Arti3DFormat i_fmtFormat);

public:

	Arti3DResult Clear();
};


#endif