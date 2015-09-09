#ifndef _ARTI3D_VERTEXLAYOUT_H_
#define _ARTI3D_VERTEXLAYOUT_H_

#include "Arti3D_Types.h"

class Arti3DVertexLayout
{
protected:
	friend class Arti3DDevice;
	Arti3DVertexLayout();
	~Arti3DVertexLayout();

	Arti3DResult Create(uint32_t iAttribute, Arti3DVertexAttributeFormat *pFormats);


public:

	Arti3DResult	iGetAttributeNum(uint32_t *o_iAttribute);
	Arti3DResult	iGetFloats(uint32_t *o_iFloat);


private:

	uint32_t						m_iVertexAttribute;
	uint32_t						m_iFloats;
	Arti3DVertexAttributeFormat		*m_pVertexAttributeFormat;
	
};

#endif