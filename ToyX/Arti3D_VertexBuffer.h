#ifndef _ARTI3D_VERTEXBUFFER_H_
#define _ARTI3D_VERTEXBUFFER_H_

#include "Arti3D_Types.h"

class Arti3DVertexBuffer
{
protected:
	friend class Arti3DDevice;

	Arti3DVertexBuffer();

	~Arti3DVertexBuffer();

	Arti3DResult	Create(uint32_t iLength);

public:
	Arti3DDevice*	pGetDevice();
	Arti3DResult	GetPointer(uint32_t i_iOffset, void **o_ppData);
	uint32_t		iGetLength();

private:
	char			*m_pData;
	uint32_t		m_iLength;
	uint32_t		m_iStride;
	Arti3DDevice	*m_pParent;
};

#endif