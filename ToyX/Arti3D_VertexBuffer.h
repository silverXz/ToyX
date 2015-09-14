#ifndef _ARTI3D_VERTEXBUFFER_H_
#define _ARTI3D_VERTEXBUFFER_H_

#include "Arti3D_Types.h"

class Arti3DVertexBuffer
{
protected:
	friend class Arti3DDevice;

	Arti3DVertexBuffer();

	~Arti3DVertexBuffer();

	// Create ( To Be More Specific, Allocate ) Vertex Buffer Storage.
	// @param iLength : Storage In Bytes.
	Arti3DResult	Create(uint32_t iLength);

public:

	// Get Parent Device Who Created This Instance.
	Arti3DDevice*	pGetDevice();

	// Get Raw Pointer To Data In The Vertex Buffer.
	// @param i_iOffset : Offset Bytes From The Beginning Of The Buffer.
	// @param o_ppData : Pointer To Target Pointer Pointer.
	Arti3DResult	GetPointer(uint32_t i_iOffset, void **o_ppData);

	// Get The Storage Bytes Of This Vertex Buffer.
	// @return : Storage In Bytes.
	uint32_t		iGetLength();

private:

	char			*m_pData;		// Pointer To Buffer Data.
	uint32_t		m_iLength;		// Storage In Bytes.
	uint32_t		m_iStride;		// The Stride Of The Vertex In The Buffer.

	Arti3DDevice	*m_pParent;		// Pointer To The Device Who Created This Instance.
};

#endif