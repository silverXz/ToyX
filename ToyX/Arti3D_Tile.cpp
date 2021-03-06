#include "stdafx.h"
#include "Arti3D_Tile.h"


Arti3DTile::Arti3DTile() : m_iX(0),
	m_iY(0),
	m_iWidth(0),
	m_iHeight(0),
	m_ppFaceIndexBuffer(nullptr),
	m_pIndexBufferSize(nullptr),
	m_ppTileCoverage(nullptr),
	m_bFinishedRasterization(false)
{

}

Arti3DTile::~Arti3DTile()
{
	if (m_ppFaceIndexBuffer)
	{
		for (int i = 0; i < ARTI3D_MAX_THREAD; ++i)
			SAFE_DELETE_ARRAY(m_ppFaceIndexBuffer[i]);
		SAFE_DELETE_ARRAY(m_ppFaceIndexBuffer);
	}

	SAFE_DELETE_ARRAY(m_pIndexBufferSize);

	if (m_ppTileCoverage)
	{
		for (int i = 0; i < ARTI3D_MAX_THREAD; ++i)
			SAFE_DELETE_ARRAY(m_ppTileCoverage[i]);
		SAFE_DELETE_ARRAY(m_ppTileCoverage);
	}
}

Arti3DResult Arti3DTile::Create(uint32_t i_iX, uint32_t i_iY, uint32_t i_iWidth, uint32_t i_iHeight)
{
	m_iX = i_iX;
	m_iY = i_iY;

	m_iWidth = i_iWidth;
	m_iHeight = i_iHeight;

	m_ppFaceIndexBuffer = new uint32_t *[ARTI3D_MAX_THREAD];
	if (!m_ppFaceIndexBuffer)
		return ARTI3D_OUT_OF_MEMORY;
	for (int i = 0; i < ARTI3D_MAX_THREAD; ++i)
	{
		m_ppFaceIndexBuffer[i] = new uint32_t[ARTI3D_MAX_FACE_PER_TILE];
		if (!m_ppFaceIndexBuffer)
			return ARTI3D_OUT_OF_MEMORY;
	}

	m_pIndexBufferSize = new uint32_t[ARTI3D_MAX_THREAD];
	if (!m_pIndexBufferSize)
		return ARTI3D_OUT_OF_MEMORY;
	memset(m_pIndexBufferSize, 0, sizeof(uint32_t)*ARTI3D_MAX_THREAD);

	m_ppTileCoverage = new Arti3DTileCoverage*[ARTI3D_MAX_THREAD];
	if (!m_ppTileCoverage)
		return ARTI3D_OUT_OF_MEMORY;
	for (int i = 0; i < ARTI3D_MAX_THREAD; ++i)
	{
		m_ppTileCoverage[i] = new Arti3DTileCoverage[ARTI3D_MAX_FACE_PER_TILE];
		if (!m_ppTileCoverage[i])
			return ARTI3D_OUT_OF_MEMORY;
	}

	m_bAddedToJobQueue.clear();

	return ARTI3D_OK;
}

void Arti3DTile::SetReadyForFragmentProcessing()
{
	m_bFinishedRasterization = true;
}

void Arti3DTile::Clear()
{
	memset(m_pIndexBufferSize, 0, sizeof(uint32_t)*ARTI3D_MAX_THREAD);
	m_bAddedToJobQueue.clear();
	m_bFinishedRasterization = false;
	m_vFragments.clear();
}
