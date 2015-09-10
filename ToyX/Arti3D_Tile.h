#ifndef _ARTI3D_TILE_H_
#define _ARTI3D_TILE_H_

#include "Arti3D_Types.h"

class Arti3DTile
{
protected:
	friend class Arti3DDevice;
	friend class Arti3DThread;

	Arti3DTile();
	~Arti3DTile();


	Arti3DResult Create();



private:
	int m_iWidth, m_iHeight;
	int	m_iX, m_iY;

	uint32_t			**m_ppFaceIndexBuffer;
	uint32_t			*m_pIndexBufferSize;
	Arti3DTileCoverage	**m_ppTileCoverage;
};


#endif