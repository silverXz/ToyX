#include "stdafx.h"
#include "Arti3D_Thread.h"


Arti3DThread::Arti3DThread() : m_pVertexCache(nullptr),
	m_pTransformedFace(nullptr),
	m_iTransformedFace(0)
{

}

Arti3DThread::~Arti3DThread()
{
	SAFE_DELETE_ARRAY(m_pVertexCache);
	SAFE_DELETE_ARRAY(m_pTransformedFace);
}

void Arti3DThread::ClearCache()
{
	m_iTransformedFace = 0;
}

Arti3DResult Arti3DThread::Create()
{
	m_pVertexCache = new Arti3DVertexCache[g_ciCacheSize];
	if (!m_pVertexCache)
		return ARTI3D_OUT_OF_MEMORY;

	m_pTransformedFace = new Arti3DTransformedFace[g_ciMaxClipVertexNumPerPatch/(g_ciMaxThreadNum * 3)];
	if (!m_pTransformedFace)
		return ARTI3D_OUT_OF_MEMORY;
	return ARTI3D_OK;
}

unsigned int Arti3DThread::WorkFunc(void *pParam)
{
	Arti3DThread *pThread = (Arti3DThread*)pParam;
	
	return 0;
}
