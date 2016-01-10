#include "stdafx.h"
#include "Arti3D_VertexBuffer.h"

Arti3DVertexLayout::Arti3DVertexLayout() : m_iVertexAttribute(0),
m_iFloats(0),
m_pVertexAttributeFormat(nullptr)
{

}

Arti3DVertexLayout::~Arti3DVertexLayout()
{
	SAFE_DELETE_ARRAY(m_pVertexAttributeFormat);
}

Arti3DResult Arti3DVertexLayout::Create(uint32_t iAttribute, Arti3DVertexAttributeFormat *pFormats)
{
	if (iAttribute == 0 || nullptr == pFormats)
		return ARTI3D_INVALID_PARAMETER;

	m_pVertexAttributeFormat = new Arti3DVertexAttributeFormat[iAttribute];

	if (!m_pVertexAttributeFormat)
		return ARTI3D_OUT_OF_MEMORY;

	memcpy(m_pVertexAttributeFormat, pFormats, sizeof(Arti3DVertexAttributeFormat) * iAttribute);

	m_iVertexAttribute = iAttribute;

	for (uint32_t i = 0; i < iAttribute; ++i)
	{
		switch (m_pVertexAttributeFormat[i])
		{
		case ARTI3D_VAF_VECTOR4:	m_iFloats += 4;	break;
		case ARTI3D_VAF_VECTOR3:	m_iFloats += 3;	break;
		case ARTI3D_VAF_VECTOR2:	m_iFloats += 2;	break;
		case ARTI3D_VAF_FLOAT32:	m_iFloats += 1; break;
		default:	return ARTI3D_INVALID_ENUM;
		}
	}

	if (m_iFloats > ARTI3D_MAX_VARYING)
		return ARTI3D_VARYING_EXCEED;

	return ARTI3D_OK;
}

Arti3DResult Arti3DVertexLayout::iGetAttributeNum(uint32_t *o_iAttribute) const
{
	if (!o_iAttribute)
		return ARTI3D_INVALID_PARAMETER;

	*o_iAttribute = m_iVertexAttribute;

	return ARTI3D_OK;
}

uint32_t Arti3DVertexLayout::iGetFloats()
{
	return m_iFloats;
}

Arti3DResult Arti3DVertexLayout::fmtGetVertexAttributeFormate(uint32_t i_iAttributeIndex, Arti3DVertexAttributeFormat *o_pVertexAttributeFormat) const
{
	if (i_iAttributeIndex >= m_iVertexAttribute)
		return ARTI3D_RANGE_EXCEED;
	*o_pVertexAttributeFormat = m_pVertexAttributeFormat[i_iAttributeIndex];
	return ARTI3D_OK;
}





Arti3DVertexBuffer::Arti3DVertexBuffer() : m_pData(nullptr), 
	m_iLength(0),
	m_iStride(0),
	m_pVertexLayout(nullptr),
	m_pParent(nullptr)
{

}

Arti3DVertexBuffer::~Arti3DVertexBuffer()
{
	SAFE_DELETE_ARRAY(m_pData);
	SAFE_DELETE(m_pVertexLayout);
}

Arti3DDevice* Arti3DVertexBuffer::pGetDevice()
{
	return m_pParent;
}

uint32_t Arti3DVertexBuffer::iGetLength()
{
	return m_iLength;
}

uint32_t Arti3DVertexBuffer::iGetStride()
{
	return m_iStride;
}

const Arti3DVertexLayout* Arti3DVertexBuffer::pGetLayout() const
{
	return m_pVertexLayout;
}

Arti3DResult Arti3DVertexBuffer::GetPointer(uint32_t i_iOffset, void **o_ppData)
{
	if (*o_ppData)
		return ARTI3D_INVALID_PARAMETER;

	if (i_iOffset >= m_iLength)
		return ARTI3D_RANGE_EXCEED;
	
	*o_ppData = m_pData + i_iOffset;
	
	return ARTI3D_OK;
}

Arti3DResult Arti3DVertexBuffer::Create(Arti3DVertexLayout *pVertexLayout, uint32_t iVertexCount)
{
	if (!iVertexCount || !pVertexLayout)
		return ARTI3D_INVALID_PARAMETER;

	m_pVertexLayout = pVertexLayout;

	m_iStride = pVertexLayout->iGetFloats() * sizeof(float);
	
	m_iLength = iVertexCount * m_iStride;

	m_pData = new char[m_iLength];

	if (!m_pData)
		return ARTI3D_OUT_OF_MEMORY;

	return ARTI3D_OK;
}


