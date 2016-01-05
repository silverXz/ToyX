#ifndef _ARTI3D_RENDERTARGET_H_
#define _ARTI3D_RENDERTARGET_H_

#include "Arti3D_Types.h"
#include "Arti3D_ForwardDecl.h"

class Arti3DRenderTarget
{
protected:

	friend class Arti3DDevice;
	friend class Arti3DThread;

	Arti3DRenderTarget(Arti3DDevice *i_pParent);
	~Arti3DRenderTarget();


public:

	// Retrieve The Pointer To The Arti3DDevice Who Created This Instance.
	Arti3DDevice* pGetDevice();


	void SetBackBuffer(Arti3DSurface *pSurface);

	void SetZBuffer(Arti3DSurface *pSurface);

	Arti3DSurface*	pGetBackBuffer() const;

	Arti3DSurface*	pGetZBuffer() const;


	Arti3DResult ClearColorBuffer(uint32_t color);

	Arti3DResult ClearDepthBuffer();

private:
	Arti3DDevice	*m_pParent;

	Arti3DSurface	*m_pBackbuffer;
	Arti3DSurface	*m_pZBuffer;
};



#endif