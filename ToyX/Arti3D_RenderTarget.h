#ifndef _ARTI3D_RENDERTARGET_H_
#define _ARTI3D_RENDERTARGET_H_

#include "Arti3D_Types.h"


class Arti3DRenderTarget
{
protected:

	friend class Arti3DDevice;

	Arti3DRenderTarget();
	~Arti3DRenderTarget(Arti3DDevice *i_pParent);


public:

	// Retrieve The Pointer To The Arti3DDevice Who Created This Instance.
	Arti3DDevice* pGetDevice();

	Arti3DResult ClearColorBuffer();

};



#endif