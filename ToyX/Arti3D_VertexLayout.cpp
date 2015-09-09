#include "stdafx.h"
#include "Arti3D_VertexLayout.h"


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

	for (int i = 0; i < iAttribute; ++i)
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

	if (m_iFloats > g_ciMaxVaryingNum)
		return ARTI3D_VARYING_EXCEED;

	return ARTI3D_OK;
}

Arti3DResult Arti3DVertexLayout::iGetAttributeNum(uint32_t *o_iAttribute)
{
	if (!o_iAttribute)
		return ARTI3D_INVALID_PARAMETER;
	
	*o_iAttribute = m_iVertexAttribute;
	
	return ARTI3D_OK;
}

Arti3DResult Arti3DVertexLayout::iGetFloats(uint32_t *o_iFloat)
{
	if (!o_iFloat)
		return ARTI3D_NULL_PARAMETER;
	*o_iFloat = m_iFloats;
	return ARTI3D_OK;
}
