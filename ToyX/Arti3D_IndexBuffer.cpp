#include "stdafx.h"
#include "Arti3D_IndexBuffer.h"


Arti3DIndexBuffer::Arti3DIndexBuffer() : m_pData(nullptr),
	m_iLength(0),
	m_fmtFormat(ARTI3D_INDEX32),
	m_pParent(nullptr)
{

}

Arti3DIndexBuffer::~Arti3DIndexBuffer()
{
	SAFE_DELETE_ARRAY(m_pData);
}

Arti3DResult Arti3DIndexBuffer::Create(uint32_t iLength, Arti3DFormat format)
{
	if (0 == iLength)
		return ARTI3D_INVALID_PARAMETER;

	m_pData = new char[iLength];

	if (!m_pData)
		return ARTI3D_OUT_OF_MEMORY;

	m_iLength = iLength;
	m_fmtFormat = format;

	return ARTI3D_OK;
}

uint32_t Arti3DIndexBuffer::iGetLength()
{
	return m_iLength;
}

Arti3DResult Arti3DIndexBuffer::GetPointer(uint32_t i_iOffset, void **o_pData)
{
	if (!o_pData)
		return ARTI3D_NULL_PARAMETER;

	if (i_iOffset >= m_iLength)
		return ARTI3D_RANGE_EXCEED;

	*o_pData = m_pData + i_iOffset;

	return ARTI3D_OK;
}
