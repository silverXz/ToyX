#ifndef _ARTI3D_INDEXBUFFER_H_
#define _ARTI3D_INDEXBUFFER_H_

#include "Arti3D_Types.h"

class Arti3DIndexBuffer
{
protected:
	friend class Arti3DDevice;

	Arti3DIndexBuffer();
	~Arti3DIndexBuffer();


	Arti3DResult Create(uint32_t iLength, Arti3DFormat format);
	

public:
	
	
	// Get The Length Of The Index Buffer In Bytes.
	uint32_t iGetLength();

	// Get The Number Of Index In The Index Buffer.
	uint32_t iGetIndexNum();

	Arti3DResult	GetPointer(uint32_t i_iOffset, void **o_pData);

	Arti3DFormat	fmtGetFormat();

	Arti3DResult	GetVertexIndex(uint32_t i_iArrayIndex, uint32_t *o_pVertexIndex);

private:
	char			*m_pData;
	uint32_t		m_iLength;
	Arti3DFormat	m_fmtFormat;
	Arti3DDevice	*m_pParent;
};


#endif