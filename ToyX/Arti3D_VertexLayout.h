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
	
	// Get The Number Of Attributes Of This Vertex.
	// @param o_iAttribute : Pointer To The Output Variable.
	Arti3DResult	iGetAttributeNum(uint32_t *o_iAttribute);
	
	// Retrieve The Vertex Format Of The Attribute. 
	// The Result Coulbe Be ARTI3D_FLOAT32, ARTI3D_VECTOR2, etc.
	// @param i_iAttributeIndex : The Index Of Target Attribute.
	// @param o_pVertexAttributeFormat : Pointer To The Output Variable.
	Arti3DResult 	fmtGetVertexAttributeFormate(uint32_t i_iAttributeIndex, Arti3DVertexAttributeFormat *o_pVertexAttributeFormat);

	// Retrieve The Total Number Of Float In The Layout.
	// E.g. VertexLayout : 2 Attributes. Format : {ARTI3D_FLOAT32, ARTI3D_VECTOR4}
	// The Function Should Return 1 + 4 = 5;
	uint32_t		iGetFloats();


private:

	uint32_t				m_iVertexAttribute;
	uint32_t				m_iFloats;
	Arti3DVertexAttributeFormat		*m_pVertexAttributeFormat;
	
};

#endif
