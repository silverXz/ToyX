#ifndef _ARTI3D_THREAD_H_
#define _ARTI3D_THREAD_H_

#include "Arti3D_Types.h"

class Arti3DThread
{
protected:

	friend class Arti3DDevice;

	Arti3DThread();
	~Arti3DThread();


public:
	void ClearCache();

	Arti3DResult Create(Arti3DDevice *pParent);

	void DistributeWorkLoad(uint32_t iThread, uint32_t iStart, uint32_t iEnd);

	static unsigned int WorkFunc(void *pParam);

private:
	void ProcessVertex();
	void PostProcessVertex(Arti3DVSOutput *io_pVSOutput);

	void PreProcessTile();

	void GetTransformedVertex(uint32_t i_iVertexIndex, Arti3DVSOutput *o_pVSOutput);
	void ClipTriangle(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3);
	void AddTransformedFace(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3);
	
	inline void ComputeTriangleGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, toy::vec2 *o_pVec2);

	void RasterizeTile();

private:
	Arti3DVertexCache		*m_pVertexCache;
	Arti3DTransformedFace	*m_pTransformedFace;
	uint32_t				m_iTransformedFace;
	uint32_t				m_iThread;
	uint32_t				m_iStart, m_iEnd;

	Arti3DDevice			*m_pParent;
};


#endif