#ifndef _ARTI3D_THREAD_H_
#define _ARTI3D_THREAD_H_

#include "Arti3D_Types.h"

class Arti3DThread
{
protected:

	friend class Arti3DDevice;

	Arti3DThread();
	~Arti3DThread();


public:
	void ClearCache();


	Arti3DResult Create();
	
	static unsigned int WorkFunc(void *pParam);

private:
	Arti3DVertexCache		*m_pVertexCache;
	Arti3DTransformedFace	*m_pTransformedFace;
	uint32_t				m_iTransformedFace;
};


#endif