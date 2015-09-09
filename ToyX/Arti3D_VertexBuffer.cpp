#include "stdafx.h"
#include "Arti3D_VertexBuffer.h"


Arti3DVertexBuffer::Arti3DVertexBuffer() : m_pData(nullptr), 
	m_iLength(0),
	m_iStride(0),
	m_pParent(nullptr)
{

}

Arti3DVertexBuffer::~Arti3DVertexBuffer()
{
	SAFE_DELETE_ARRAY(m_pData);
}

Arti3DDevice* Arti3DVertexBuffer::pGetDevice()
{
	return m_pParent;
}

uint32_t Arti3DVertexBuffer::iGetLength()
{
	return m_iLength;
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

Arti3DResult Arti3DVertexBuffer::Create(uint32_t iLength)
{
	if (!iLength)
		return ARTI3D_INVALID_PARAMETER;

	m_pData = new char[iLength];

	if (!m_pData)
		return ARTI3D_OUT_OF_MEMORY;
	
	m_iLength = iLength;

	return ARTI3D_OK;
	
}



