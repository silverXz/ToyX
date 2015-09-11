#ifndef _ARTI3D_TILE_H_
#define _ARTI3D_TILE_H_

#include <atomic>
#include <vector>

#include "Arti3D_Types.h"

class Arti3DTile
{
protected:
	friend class Arti3DDevice;
	friend class Arti3DThread;

	Arti3DTile();
	~Arti3DTile();


	Arti3DResult Create();

	void SetReadyForFragmentProcessing();

	void Clear();


private:
	int m_iWidth, m_iHeight;
	int	m_iX, m_iY;

	uint32_t			**m_ppFaceIndexBuffer;
	uint32_t			*m_pIndexBufferSize;
	Arti3DTileCoverage	**m_ppTileCoverage;

	std::vector<Arti3DFragment> m_vFragments;
	std::atomic_flag	m_bAddedToJobQueue;
	bool				m_bFinishedRasterization;
};


#endif