#ifndef _ARTI3D_VERTEXBUFFER_H_
#define _ARTI3D_VERTEXBUFFER_H_

#include "Arti3D_Types.h"

// Arti3D Vertex Layout Declaration.
class Arti3DVertexLayout
{
protected:
	friend class Arti3DDevice;
	Arti3DVertexLayout();

	// Create Vertex Layout.
	// @param iAttribute : The Number Of Attributes Of The Vertex. By Attributes, I Mean Position, Normal, Texture Coordinates, etc.
	// @param pFormats : Pointer To An Array Of Attribute Formats. The Size Of The Array Mustn't Less Than "iAttribute".
	Arti3DResult Create(uint32_t iAttribute, Arti3DVertexAttributeFormat *pFormats);

public:
	~Arti3DVertexLayout();
public:

	// Get The Number Of Attributes Of This Vertex.
	// @param o_iAttribute : Pointer To The Output Variable.
	Arti3DResult	iGetAttributeNum(uint32_t *o_iAttribute) const;

	// Retrieve The Vertex Format Of The Attribute. 
	// The Result Coulbe Be ARTI3D_FLOAT32, ARTI3D_VECTOR2, etc.
	// @param i_iAttributeIndex : The Index Of Target Attribute.
	// @param o_pVertexAttributeFormat : Pointer To The Output Variable.
	Arti3DResult 	fmtGetVertexAttributeFormate(uint32_t i_iAttributeIndex, Arti3DVertexAttributeFormat *o_pVertexAttributeFormat) const;

	// Retrieve The Total Number Of Float In The Layout.
	// E.g. VertexLayout : 2 Attributes. Format : {ARTI3D_FLOAT32, ARTI3D_VECTOR4}
	// The Function Should Return 1 + 4 = 5;
	uint32_t		iGetFloats();
	
private:
	uint32_t						m_iVertexAttribute;				// Number Of Vertex Attribute.
	uint32_t						m_iFloats;						// Totoal Number Of Floats Of All Attributes.
	Arti3DVertexAttributeFormat		*m_pVertexAttributeFormat;		// Pointer To Vertex Attribute Format.
};


// Arti3D Vertex Buffer Declaration.
class Arti3DVertexBuffer
{
protected:
	friend class Arti3DDevice;

	Arti3DVertexBuffer();



	// Create ( To Be More Specific, Allocate ) Vertex Buffer Storage.
	// @param  pVertexLayout.
	// @param  iVertexCount: 
	Arti3DResult	Create(Arti3DVertexLayout *pVertexLayout,uint32_t iVertexCount);

public:
	~Arti3DVertexBuffer();
	
	// Get Parent Device Who Created This Instance.
	Arti3DDevice*	pGetDevice();

	// Get Raw Pointer To Data In The Vertex Buffer.
	// @param i_iOffset : Offset Bytes From The Beginning Of The Buffer.
	// @param o_ppData : Pointer To Target Pointer Pointer.
	Arti3DResult	GetPointer(uint32_t i_iOffset, void **o_ppData);

	// Get The Storage Bytes Of This Vertex Buffer.
	// @return : Storage In Bytes.
	uint32_t		iGetLength();

	uint32_t		iGetStride();

	const Arti3DVertexLayout* pGetLayout() const;

private:

	char			*m_pData;		// Pointer To Buffer Data.
	uint32_t		m_iLength;		// Storage In Bytes.
	uint32_t		m_iStride;		// The Stride Of The Vertex In The Buffer.

	Arti3DVertexLayout	*m_pVertexLayout;
	Arti3DDevice		*m_pParent;		// Pointer To The Device Who Created This Instance.
};

#endif