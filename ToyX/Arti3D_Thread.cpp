#include "stdafx.h"
#include <algorithm>
#include <atomic>
#include <thread>
#include <chrono>
#include "Arti3D_Thread.h"
#include "Arti3D_Device.h"
#include "Arti3D_IndexBuffer.h"
#include "Arti3D_VertexBuffer.h"
#include "Arti3D_VertexLayout.h"
#include "Arti3D_RenderTarget.h"
#include "Arti3D_Surface.h"
#include "Arti3D_Tile.h"
#include "Arti3D_ShaderBase.h"


Arti3DThread::Arti3DThread() : m_pVertexCache(nullptr),
	m_pTransformedFace(nullptr),
	m_iTransformedFace(0),
	m_iThread(0),
	m_iStart(0),
	m_iEnd(0),
	m_pParent(nullptr)
{

}

Arti3DThread::~Arti3DThread()
{
	SAFE_DELETE_ARRAY(m_pVertexCache);
	SAFE_DELETE_ARRAY(m_pTransformedFace);
}

void Arti3DThread::ClearCacheAndBuffer()
{
	m_iTransformedFace = 0;

	if (m_pVertexCache)
	{
		for (int i = 0; i < ARTI3D_MAX_CACHE_SIZE; ++i)
			m_pVertexCache[i].Clear();
	}
}
Arti3DResult Arti3DThread::Create(Arti3DDevice *pParent, uint32_t iThread)
{
	m_pVertexCache = new Arti3DVertexCache[ARTI3D_MAX_CACHE_SIZE];
	if (!m_pVertexCache)
		return ARTI3D_OUT_OF_MEMORY;

	m_pTransformedFace = new Arti3DTransformedFace[ARTI3D_MAX_CLIP_VERTEX_PER_PATCH/(ARTI3D_MAX_THREAD * 3)];
	if (!m_pTransformedFace)
		return ARTI3D_OUT_OF_MEMORY;

	m_iThread = iThread;
	m_pParent = pParent;

	return ARTI3D_OK;
}

void Arti3DThread::WorkFunc(Arti3DThread *pThread)
{
	Arti3DDevice *pDev = pThread->m_pParent;

	while (pThread->m_pParent->m_iStage == 1)
		std::this_thread::yield();

	while (!pDev->m_bThreadStop)
	{
		pThread->ClearCacheAndBuffer();

		pThread->ProcessVertex();
		
		pThread->PreProcessTile();

		// Till Now, All Triangle Has Been Processed And Added To JobQueue.
		// Next, We What To Fetch Tile From JobQueue And Rasterize Tile To Generate Fragments.
		// But!!!!! What Have To Wait For All Thread Finishing What We Have Done Now.
		
		// All Thread Are Done For The First Stage!
		
		if (--pDev->m_iWorkingThread == 0)
		{
			pDev->m_iWorkingThread = ARTI3D_MAX_THREAD;
			pDev->m_iStage = 1;
		} 

		while (pDev->m_iStage == 0)
			std::this_thread::yield();
		
		// OK,Then.Let's Take Things To The Next Level! 
		uint32_t iJob = 0;
		while ((iJob = pDev->m_iJobStart++) < pDev->m_iJobEnd)
		{
			Arti3DTile *pTile = &pDev->m_pTiles[pDev->m_pJobQueue[iJob]];
			pThread->RasterizeTile(pTile);
			pTile->m_bFinishedRasterization = true;
		}
		
		// This Thread Finishes Rasterization, Start Fragment Processing.
		while ((iJob = pDev->m_iJobStart2++) < pDev->m_iJobEnd)
		{
			Arti3DTile *pTile = &pDev->m_pTiles[pDev->m_pJobQueue[iJob]];
			while (!pTile->m_bFinishedRasterization)
				std::this_thread::yield();
			pThread->RenderFragment(pTile);
		}

		--pDev->m_iWorkingThread;
		
		while (pDev->m_iStage == 1 && !pDev->m_bThreadStop)
			std::this_thread::yield();
	}
	printf("Thread #%d returns.\n", pThread->m_iThread);
	return;
}

void Arti3DThread::ProcessVertex()
{

	for (uint32_t i = m_iStart; i < m_iEnd; i+=3)
	{
		Arti3DVSOutput v[ARTI3D_MAX_CLIP_VERTEX];

		for (int j = 0; j < 3; ++j)
		{
			uint32_t iVertexIndex = 0;
			m_pParent->m_pIndexBuffer->GetVertexIndex(i + j, &iVertexIndex);
			GetTransformedVertex(iVertexIndex, &v[j]);
		}
		ClipTriangle(&v[0], &v[1], &v[2]);
	}
}

void Arti3DThread::DistributeWorkLoad(uint32_t iThread, uint32_t iStart, uint32_t iEnd)
{
	m_iThread = iThread;
	m_iStart = iStart;
	m_iEnd = iEnd;
}

void Arti3DThread::GetTransformedVertex(uint32_t i_iVertexIndex, Arti3DVSOutput *o_pVSOutput)
{
	// Calculate cache index for this vertex.
	uint32_t iCacheIndex = i_iVertexIndex&(ARTI3D_MAX_CACHE_SIZE - 1);

	// Try to fetch result from cache first.
	if (m_pVertexCache[iCacheIndex].tag == i_iVertexIndex)
	{
		*o_pVSOutput = m_pVertexCache[iCacheIndex].vs_output;
	}
	else
	{
		// Cache miss. Calculate!

		// Setup input for vertex shader.
		Arti3DVSInput vsinput;

		void *pSrc = nullptr;
		m_pParent->m_pVertexBuffer->GetPointer(m_pParent->m_pVertexLayout->iGetFloats() * sizeof(float) * i_iVertexIndex, &pSrc);
		float *pV = (float*)pSrc;

		uint32_t iAttributeNum = 0;
		m_pParent->m_pVertexLayout->iGetAttributeNum(&iAttributeNum);

		Arti3DVertexAttributeFormat fmtVertexAttribute;
		for (uint32_t i = 0; i < iAttributeNum; ++i)
		{
			m_pParent->m_pVertexLayout->fmtGetVertexAttributeFormate(i, &fmtVertexAttribute);
			switch (fmtVertexAttribute)
			{
			case ARTI3D_VAF_VECTOR4:
				vsinput.ShaderInputs[i] = ShaderRegister(pV[0], pV[1], pV[2], pV[3]);	pV += 4;	break;
			case ARTI3D_VAF_VECTOR3:
				vsinput.ShaderInputs[i] = ShaderRegister(pV[0], pV[1], pV[2], 0.0f);	pV += 3;	break;
			case ARTI3D_VAF_VECTOR2:
				vsinput.ShaderInputs[i] = ShaderRegister(pV[0], pV[1], 0.0f, 0.0f);		pV += 2;	break;
			case ARTI3D_VAF_FLOAT32:
				vsinput.ShaderInputs[i] = ShaderRegister(pV[0]);						pV += 1;	break;
			default:break;
			}
		}

		// Execute vertex shader.
		// Save results to cache.
		m_pParent->mRC.pVertexShader->Execute(&vsinput, &m_pParent->mRC.globals, &m_pVertexCache[iCacheIndex].vs_output);
		m_pVertexCache[iCacheIndex].tag = i_iVertexIndex;
		*o_pVSOutput = m_pVertexCache[iCacheIndex].vs_output;
	}
}

void Arti3DThread::ClipTriangle(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3)
{
	auto calcClipMask = [](Arti3DVSOutput *v) {
		int mask = 0;
		if (v->p.x - v->p.w > 0) mask |= CLIP_POS_X;
		if (v->p.x + v->p.w < 0) mask |= CLIP_NEG_X;
		if (v->p.y - v->p.w > 0) mask |= CLIP_POS_Y;
		if (v->p.y + v->p.w < 0) mask |= CLIP_NEG_Y;
		if (v->p.z - v->p.w > 0) mask |= CLIP_POS_Z;
		if (v->p.z + v->p.w < 0) mask |= CLIP_NEG_Z;
		return mask;
	};

	int mask = 0;
	mask |= calcClipMask(v1);
	mask |= calcClipMask(v2);
	mask |= calcClipMask(v3);

	// Totally inside!
	if (mask == 0x0)
	{
		PostProcessVertex(v1);
		PostProcessVertex(v2);
		PostProcessVertex(v3);
		AddTransformedFace(v1, v2, v3);
		return;
	}

	// Clipping happens! Do the clip work!
	Arti3DPlane p[6] = {
		{ -1.0f, 0.0f, 0.0f, 1.0f },	// POS_X_PLANE ( pointing at -x )
		{ 1.0f, 0.0f, 0.0f, 1.0f },		// NEG_X_PLANE ( pointing at +x )
		{ 0.0f, -1.0f, 0.0f, 1.0f },	// POS_Y_PLANE ( pointing at -y )
		{ 0.0f, 1.0f, 0.0f, 1.0 },		// NEG_Y_PLANE ( pointing at +y )
		{ 0.0f, 0.0f, -1.0f, 1.0f },	// POS_Z_PLANE ( pointing at -z )
		{ 0.0f, 0.0f, 1.0f, 0.0f }		// NEG_Z_PLANE ( pointing at +z )
	};

	Arti3DClipMask mk[6] = {
		CLIP_POS_X,
		CLIP_NEG_X,
		CLIP_POS_Y,
		CLIP_NEG_Y,
		CLIP_POS_Z,
		CLIP_NEG_Z
	};

	// We need 2 index array ( with size CLIP_VERTEX_MAX ) to do ping pong buffering.
	Arti3DVSOutput *v = v1;
	uint32_t inout[2][ARTI3D_MAX_CLIP_VERTEX];
	uint32_t *in = inout[0], *out = inout[1];
	in[0] = 0;	in[1] = 1; in[2] = 2;

	int vCnt = 3;
	int inCnt = 3;
	int outCnt = 0;

	// Several lambda expression to help!

	// Calculate the signed distance between a vertex and a plane.
	auto calcPointPlaneDistance = [](const Arti3DPlane *p, const Arti3DVSOutput *v) { 	return p->x * v->p.x + p->y * v->p.y + p->z * v->p.z + p->d * v->p.w; };

	// Determine whether two floats has different signs.
	auto hasDifferentSigns = [](float a, float b) { return (a >= 0.0f && b < 0.0f) || (a < 0.0f && b >= 0.0f);	};

	// Interpolate Vertex Attributes
	auto interpolateV = [](const Arti3DVSOutput *v1, const Arti3DVSOutput *v2, float t, Arti3DVSOutput *out) {
		out->p.x = v1->p.x + (v2->p.x - v1->p.x) * t;
		out->p.y = v1->p.y + (v2->p.y - v1->p.y) * t;
		out->p.z = v1->p.z + (v2->p.z - v1->p.z) * t;
		out->p.w = v1->p.w + (v2->p.w - v1->p.w) * t;
		for (int i = 0; i < ARTI3D_MAX_VARYING; ++i)
			out->varyings[i] = v1->varyings[i] + (v2->varyings[i] - v1->varyings[i]) * t;
	};

	// For every clip plane, find the clipped vertices and put their index in "out" array.
	for (int i = 0; i < 6; ++i)
	{
		// There is at least one vertex clipped by plane mk[i]
		if (mask & mk[i])
		{
			int id1 = in[inCnt] = in[0];
			float d1 = calcPointPlaneDistance(&p[i], &v[id1]);

			for (int j = 1; j <= inCnt; ++j)
			{
				int id2 = in[j];
				float d2 = calcPointPlaneDistance(&p[i], &v[id2]);

				// The first vertex of this edge is inside p[i].We should add its indice to "out" array!
				if (d1 >= 0)
					out[outCnt++] = id1;

				// Next, we have to find whether this edge is cut by p[i].
				// If yes, interpolate the cut vertex and add its indice to "out" array!
				// If no, continue.
				if (hasDifferentSigns(d1, d2))
				{
					float t = d1 / (d1 - d2);
					interpolateV(&v[id1], &v[id2], t, &v[vCnt]);
					out[outCnt++] = vCnt++;
				}

				id1 = id2;
				d1 = d2;
			}

			// All vertices are clipped out by this plane. No need to process further.
			if (outCnt < 3)
				return;

			std::swap(in, out);

			inCnt = outCnt;
			outCnt = 0;

		}


	}

	for (int i = 0; i < inCnt; ++i)
	{
		Arti3DVSOutput *p = &v[in[i]];
		PostProcessVertex(p);
	}

	for (int i = 1; i < inCnt - 1; ++i)
		AddTransformedFace(&v[in[0]], &v[in[i]], &v[in[i + 1]]);
}

void Arti3DThread::PostProcessVertex(Arti3DVSOutput *io_pVSOutput)
{
	// Perspective division.
	float invW = 1.0f / io_pVSOutput->p.w;

	io_pVSOutput->p.x *= invW;
	io_pVSOutput->p.y *= invW;
	io_pVSOutput->p.w = invW;

	for (int j = 0; j < ARTI3D_MAX_VARYING; ++j)
		io_pVSOutput->varyings[j] *= invW;
	
	// Transform to screen space.
	Arti3DSurface *pbb = m_pParent->m_pRenderTarget->m_pBackbuffer;

	int w = pbb->iGetWidth();
	int h = pbb->iGetHeight();

	io_pVSOutput->p.x = (io_pVSOutput->p.x + 1.0f) * 0.5f * w;
	io_pVSOutput->p.y = h - (io_pVSOutput->p.y + 1.0f)* 0.5f * h;
}

void Arti3DThread::AddTransformedFace(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3)
{
	Arti3DTransformedFace f;

	memset(&f, 0, sizeof(Arti3DTransformedFace));

	f.v0x = v1->p.x;
	f.v0y = v1->p.y;
	f.v0w = v1->p.w;

	for (int i = 0; i < ARTI3D_MAX_VARYING; ++i)
		f.v0v[i] = v1->varyings[i];

	f.fp1[0] = iRound(v1->p.x * 16.0f);
	f.fp1[1] = iRound(v1->p.y * 16.0f);
	f.fp2[0] = iRound(v2->p.x * 16.0f);
	f.fp2[1] = iRound(v2->p.y * 16.0f);
	f.fp3[0] = iRound(v3->p.x * 16.0f);
	f.fp3[1] = iRound(v3->p.y * 16.0f);

	float fdx21 = v2->p.x - v1->p.x;
	float fdx31 = v3->p.x - v1->p.x;
	float fdy21 = v2->p.y - v1->p.y;
	float fdy31 = v3->p.y - v1->p.y;

	float fdw21 = v2->p.w - v1->p.w;
	float fdw31 = v3->p.w - v1->p.w;

	float CS = fdx21 * fdy31 - fdx31 * fdy21;
	ComputeTriangleGradient(CS, v2->p.w - v1->p.w, v3->p.w - v1->p.w, fdx21, fdy21, fdx31, fdy31, &f.dw);

	for (int i = 0; i < ARTI3D_MAX_VARYING; ++i)
	{
		ComputeTriangleGradient(CS, v2->varyings[i] - v1->varyings[i], v3->varyings[i] - v1->varyings[i], fdx21, fdy21, fdx31, fdy31, &f.dv[i]);
	}

	m_pTransformedFace[m_iTransformedFace++] = f;
}

void Arti3DThread::ComputeTriangleGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, a3d::vec2 *o_pVec2)
{
	float A = di21 * dy31 - di31 * dy21;
	float B = di21 * dx31 - di31 * dx21;
	o_pVec2->x = A / C;
	o_pVec2->y = -B / C;
}

void Arti3DThread::PreProcessTile()
{
	Arti3DTile *pTile = m_pParent->m_pTiles;

	int iTileX = m_pParent->m_iTileX;
	int iTileY = m_pParent->m_iTileY;

	for (uint32_t iFaceID = 0; iFaceID < m_iTransformedFace; ++iFaceID)
	{
		Arti3DTransformedFace *f = &m_pTransformedFace[iFaceID];

		int DX21 = f->fp2[0] - f->fp1[0];
		int DY21 = f->fp2[1] - f->fp1[1];
		int DX32 = f->fp3[0] - f->fp2[0];
		int DY32 = f->fp3[1] - f->fp2[1];
		int DX13 = f->fp1[0] - f->fp3[0];
		int DY13 = f->fp1[1] - f->fp3[1];

		int faceOrient = DX13*DY21 - DX21*DY13;

		if (faceOrient > 0)
			continue;

		int C1 = -DY21 * f->fp1[0] + DX21 * f->fp1[1];
		int C2 = -DY32 * f->fp2[0] + DX32 * f->fp2[1];
		int C3 = -DY13 * f->fp3[0] + DX13 * f->fp3[1];

		if (DY21 > 0 || (DY21 == 0 && DX21 > 0))
			++C1;
		if (DY32 > 0 || (DY32 == 0 && DX32 > 0))
			++C2;
		if (DY13 > 0 || (DY13 == 0 && DX13 > 0))
			++C3;

		const int DDX21 = DX21 << 4;
		const int DDX32 = DX32 << 4;
		const int DDX13 = DX13 << 4;
		const int DDY21 = DY21 << 4;
		const int DDY32 = DY32 << 4;
		const int DDY13 = DY13 << 4;

		// Calculate face fixed points boundary.
		int xmin = std::min(std::min(f->fp1[0], f->fp2[0]), f->fp3[0]);
		int xmax = std::max(std::max(f->fp1[0], f->fp2[0]), f->fp3[0]);
		int ymin = std::min(std::min(f->fp1[1], f->fp2[1]), f->fp3[1]);
		int ymax = std::max(std::max(f->fp1[1], f->fp2[1]), f->fp3[1]);

		// Tile index boundary.
		int ixMinTile = ((xmin + 0xF) >> 4) >> g_ciTileSizeShift;
		int ixMaxTile = ((xmax + 0xF) >> 4) >> g_ciTileSizeShift;
		int iyMinTile = ((ymin + 0xF) >> 4) >> g_ciTileSizeShift;
		int iyMaxTile = ((ymax + 0xF) >> 4) >> g_ciTileSizeShift;

		// Wrap tile index.
		ixMaxTile = ixMaxTile >= iTileX ? iTileX - 1 : ixMaxTile;
		iyMaxTile = iyMaxTile >= iTileY ? iTileY - 1 : iyMaxTile;

		// Traversal every tile may cover this face.
		for (int y = iyMinTile; y <= iyMaxTile; ++y)
		{
			for (int x = ixMinTile; x <= ixMaxTile; ++x)
			{
				// Get Tile Corner Fix Point Coordinate.
				int iTile = y * iTileX + x;

				Arti3DTile &tile = pTile[iTile];

				// fixed point corners of this tile.
				int x0 = tile.m_iX << 4;
				int x1 = (tile.m_iX + g_ciTileSize - 1) << 4;
				int y0 = tile.m_iY << 4;
				int y1 = (tile.m_iY + g_ciTileSize - 1) << 4;

				// calculate coverage mask that "4 tile corners" against "one edge".
				// "one edge" indicated by @C,@dy,@dx.
				auto calcEdgeMask = [&](int C, int dy, int dx) {
					bool m0 = (C + dy * x0 - dx * y0) > 0;
					bool m1 = (C + dy * x0 - dx * y1) > 0;
					bool m2 = (C + dy * x1 - dx * y0) > 0;
					bool m3 = (C + dy * x1 - dx * y1) > 0;
					return (m0 << 0) | (m1 << 1) | (m2 << 2) | (m3 << 3);
				};

				// Test Tile Against 3 Edges
				int a1 = calcEdgeMask(C1, DY21, DX21);
				int a2 = calcEdgeMask(C2, DY32, DX32);
				int a3 = calcEdgeMask(C3, DY13, DX13);

				// Tile Completely Outside Triangle!
				if (a1 == 0 || a2 == 0 || a3 == 0)
					continue;

				// Tile Completely Inside Triangle
				if (a1 == 0xF && a2 == 0xF && a3 == 0xF)
				{
					if (tile.m_iWidth == g_ciTileSize && tile.m_iHeight == g_ciTileSize)
					{
						uint32_t &refIndexIndex = tile.m_pIndexBufferSize[m_iThread];
						tile.m_ppFaceIndexBuffer[m_iThread][refIndexIndex] = iFaceID;
						tile.m_ppTileCoverage[m_iThread][refIndexIndex] = ARTI3D_TC_ALL;
						++refIndexIndex;
					}
				}
				else
				{
					uint32_t &refIndexIndex = tile.m_pIndexBufferSize[m_iThread];
					tile.m_ppFaceIndexBuffer[m_iThread][refIndexIndex] = iFaceID;
					tile.m_ppTileCoverage[m_iThread][refIndexIndex] = ARTI3D_TC_PARTIAL;
					++refIndexIndex;
				}

				// If This Tile Is Not Already Added To The Job Queue, Add It And Set The Flag.
				if (!tile.m_bAddedToJobQueue.test_and_set())
				{
					m_pParent->m_pJobQueue[m_pParent->m_iJobEnd++] = iTile;
				}
			}
		}
	}
}

void Arti3DThread::RasterizeTile(Arti3DTile *io_pTile)
{
	for (int i = 0; i < ARTI3D_MAX_THREAD; ++i)
	{
		PArti3DThread tmpThread = &m_pParent->m_pThreads[i];
		
		for (uint32_t j = 0; j < io_pTile->m_pIndexBufferSize[i]; ++j)
		{
			Arti3DTileCoverage eTileCoverage = io_pTile->m_ppTileCoverage[i][j];
			uint32_t iFace = io_pTile->m_ppFaceIndexBuffer[i][j];
			if (eTileCoverage == ARTI3D_TC_ALL)
			{
				Arti3DFragment frag;
				memset(&frag, 0, sizeof(Arti3DFragment));
				frag.x = io_pTile->m_iX;
				frag.y = io_pTile->m_iY;
				frag.faceID = iFace;
				frag.threadID = i;
				frag.coverType = ARTI3D_FC_TILE;
				io_pTile->m_vFragments.push_back(frag);
				continue;
			}

			// This tile is partially covered!
			// Calculate precious coverage masks!
			Arti3DTransformedFace *f = &tmpThread->m_pTransformedFace[iFace];
			
			int DX21 = f->fp2[0] - f->fp1[0];
			int DY21 = f->fp2[1] - f->fp1[1];
			int DX32 = f->fp3[0] - f->fp2[0];
			int DY32 = f->fp3[1] - f->fp2[1];
			int DX13 = f->fp1[0] - f->fp3[0];
			int DY13 = f->fp1[1] - f->fp3[1];

			int faceOrient = DX13*DY21 - DX21*DY13;

			// Back face culling.
			if (faceOrient > 0)
				return;


			int C1 = -DY21 * f->fp1[0] + DX21 * f->fp1[1];
			int C2 = -DY32 * f->fp2[0] + DX32 * f->fp2[1];
			int C3 = -DY13 * f->fp3[0] + DX13 * f->fp3[1];

			if (DY21 > 0 || (DY21 == 0 && DX21 > 0))
				++C1;
			if (DY32 > 0 || (DY32 == 0 && DX32 > 0))
				++C2;
			if (DY13 > 0 || (DY13 == 0 && DX13 > 0))
				++C3;

			const int DDX21 = DX21 << 4;
			const int DDX32 = DX32 << 4;
			const int DDX13 = DX13 << 4;
			const int DDY21 = DY21 << 4;
			const int DDY32 = DY32 << 4;
			const int DDY13 = DY13 << 4;

			// Break this tile into blocks (8*8 pixels).
			for (int y = io_pTile->m_iY; y < io_pTile->m_iY+ io_pTile->m_iHeight; y += g_ciBlockSize)
			{
				for (int x = io_pTile->m_iX; x < io_pTile->m_iX + io_pTile->m_iWidth; x += g_ciBlockSize)
				{
					int x0 = x << 4;
					int x1 = (x + g_ciBlockSize - 1) << 4;
					int y0 = y << 4;
					int y1 = (y + g_ciBlockSize - 1) << 4;

					// Calculate coverage mask that "block corners" against "one edge".
					// "one edge" indicated by @C,@dy,@dx.
					auto calcEdgeMask = [&](int C, int dy, int dx) {
						bool m0 = (C + dy * x0 - dx * y0) > 0;
						bool m1 = (C + dy * x0 - dx * y1) > 0;
						bool m2 = (C + dy * x1 - dx * y0) > 0;
						bool m3 = (C + dy * x1 - dx * y1) > 0;
						return (m0 << 0) | (m1 << 1) | (m2 << 2) | (m3 << 3);
					};

					// Test block corners against 3 edges
					int a1 = calcEdgeMask(C1, DY21, DX21);
					int a2 = calcEdgeMask(C2, DY32, DX32);
					int a3 = calcEdgeMask(C3, DY13, DX13);

					// Block Completely Outside Triangle!
					if (a1 == 0 || a2 == 0 || a3 == 0)
						continue;

					// Block Totally Inside Triangle!
					if (a1 == 0xF && a2 == 0xF && a3 == 0xF)
					{
						Arti3DFragment frag;
						memset(&frag, 0, sizeof(Arti3DFragment));
						frag.x = x;
						frag.y = y;
						frag.coverType = ARTI3D_FC_BLOCK;
						frag.faceID = iFace;
						frag.threadID = i;
						io_pTile->m_vFragments.push_back(frag);
						continue;
					}

					// Block partially inside trianglel!
					// Calculate coverage mask!
					__m128i B1 = _mm_set1_epi32(C1 + DY21 * x0 - DX21 * y0);
					__m128i B2 = _mm_set1_epi32(C2 + DY32 * x0 - DX32 * y0);
					__m128i B3 = _mm_set1_epi32(C3 + DY13 * x0 - DX13 * y0);

					__m128i offsetDDY21 = _mm_set_epi32(3 * DDY21, 2 * DDY21, DDY21, 0);
					__m128i offsetDDY32 = _mm_set_epi32(3 * DDY32, 2 * DDY32, DDY32, 0);
					__m128i offsetDDY13 = _mm_set_epi32(3 * DDY13, 2 * DDY13, DDY13, 0);

					__m128i offsetDDY21ex = _mm_set1_epi32(4 * DDY21);
					__m128i offsetDDY32ex = _mm_set1_epi32(4 * DDY32);
					__m128i offsetDDY13ex = _mm_set1_epi32(4 * DDY13);

					__m128i offsetDDX21 = _mm_set1_epi32(DDX21);
					__m128i offsetDDX32 = _mm_set1_epi32(DDX32);
					__m128i offsetDDX13 = _mm_set1_epi32(DDX13);

					// Traversal every line (8 pixels) of the block.
					for (int k = 0; k < g_ciBlockSize; ++k)
					{
						// First 4 pixels
						__m128i E1 = _mm_add_epi32(B1, offsetDDY21);
						__m128i E2 = _mm_add_epi32(B2, offsetDDY32);
						__m128i E3 = _mm_add_epi32(B3, offsetDDY13);

						__m128i xm1 = _mm_cmpgt_epi32(E1, _mm_setzero_si128());
						__m128i xm2 = _mm_cmpgt_epi32(E2, _mm_setzero_si128());
						__m128i xm3 = _mm_cmpgt_epi32(E3, _mm_setzero_si128());

						__m128i xm = _mm_and_si128(xm1, _mm_and_si128(xm2, xm3));


						int im = _mm_movemask_ps(*(__m128*)&xm);

						if (0 != im)
						{
							Arti3DFragment frag;
							memset(&frag, 0, sizeof(Arti3DFragment));
							frag.x = x;
							frag.y = y + k;
							frag.faceID = iFace;
							frag.threadID = i;
							frag.mask = im;
							frag.coverType = ARTI3D_FC_MASKED;
							io_pTile->m_vFragments.push_back(frag);
						}

						// Second 4 pixels

						E1 = _mm_add_epi32(E1, offsetDDY21ex);
						E2 = _mm_add_epi32(E2, offsetDDY32ex);
						E3 = _mm_add_epi32(E3, offsetDDY13ex);

						xm1 = _mm_cmpgt_epi32(E1, _mm_setzero_si128());
						xm2 = _mm_cmpgt_epi32(E2, _mm_setzero_si128());
						xm3 = _mm_cmpgt_epi32(E3, _mm_setzero_si128());

						xm = _mm_and_si128(xm1, _mm_and_si128(xm2, xm3));

						im = _mm_movemask_ps(*(__m128*)&xm);

						if (0 != im)
						{
							Arti3DFragment frag;
							memset(&frag, 0, sizeof(Arti3DFragment));
							frag.x = x + 4;
							frag.y = y + k;
							frag.faceID = iFace;
							frag.threadID = i;
							frag.mask = im;
							frag.coverType = ARTI3D_FC_MASKED;
							io_pTile->m_vFragments.push_back(frag);
						}
						B1 = _mm_sub_epi32(B1, offsetDDX21);
						B2 = _mm_sub_epi32(B2, offsetDDX32);
						B3 = _mm_sub_epi32(B3, offsetDDX13);
					}
				}
			}
		}
	}
}

void Arti3DThread::RenderFragment(Arti3DTile *i_pTile)
{
	for (auto& frag : i_pTile->m_vFragments)
	{
		switch (frag.coverType)
		{
		case ARTI3D_FC_TILE:
			RenderTileFragments(&frag);
			break;
		case ARTI3D_FC_BLOCK:
			RenderBlockFragments(&frag);
			break;
		case ARTI3D_FC_MASKED:
			RenderMaskedFragments(&frag);
			break;
		default:
			break;
		}
	}
}

void Arti3DThread::RenderTileFragments(Arti3DFragment *i_pFrag)
{
	__m128 W0, W1, WDY;
	__m128 V0[ARTI3D_MAX_VARYING], V1[ARTI3D_MAX_VARYING], VDY[ARTI3D_MAX_VARYING];
	__m128 C0 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
	__m128 C1 = _mm_set_ps1(4.0f);

	PArti3DThread pThread = &m_pParent->m_pThreads[i_pFrag->threadID];
	Arti3DTransformedFace *f = &pThread->m_pTransformedFace[i_pFrag->faceID];

	Arti3DSurface *pbb = m_pParent->m_pRenderTarget->m_pBackbuffer;
	void *pixels = pbb->pGetPixelsDataPtr();
	int wp = pbb->iGetWidth();

	Arti3DSurface *pzb = m_pParent->m_pRenderTarget->m_pZBuffer;
	void *depths = pzb->pGetPixelsDataPtr();
	int wz = pzb->iGetWidth();

	PArti3DPixelShader pPixelShader = m_pParent->mRC.pPixelShader;
	
	float *depthBuffer = nullptr;
	uint32_t *colorBuffer = nullptr;

	for (int x = i_pFrag->x; x < i_pFrag->x + g_ciTileSize; x += g_ciBlockSize)
	{
		colorBuffer = (uint32_t*)pixels + i_pFrag->y * wp + x;
		depthBuffer = (float*)depths + i_pFrag->y * wz + x;

		for (int y = i_pFrag->y; y < i_pFrag->y + g_ciTileSize; ++y)
		{
			CalcVaryings(f, x, y, W0, W1, WDY, V0, V1, VDY);
			SSE_ALIGN Arti3DPSParam ps_param;
			
			__m128	dbquad;
			__m128i dbmask, oquad, nquad;
			uint32_t	*colorTileLine = colorBuffer;
			float		*depthTileLine = depthBuffer;

			dbquad = _mm_loadu_ps(depthTileLine);
			oquad = _mm_loadu_si128((__m128i*)colorTileLine);

			// Depth test
			dbmask = *(__m128i*)&_mm_cmpge_ps(W0, dbquad);

			// If At Least 1 Pixels Passed Test.
			if (_mm_movemask_ps(*(__m128*)&dbmask))
			{
				PreInterpolateVaryings(W0, V0, ps_param.Varyings);
				pPixelShader->Execute(&ps_param);

				nquad = ConvertColorFormat(ps_param.Output);

				// Update Depth Buffer!
				dbquad = _mm_or_ps(_mm_and_ps(W0, *(__m128*)&dbmask), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
				_mm_storeu_ps(depthTileLine, dbquad);

				nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
				_mm_storeu_si128((__m128i*)colorTileLine, nquad);
			}

			colorTileLine += 4;
			depthTileLine += 4;

			dbquad = _mm_loadu_ps(depthTileLine);
			oquad = _mm_loadu_si128((__m128i*)colorTileLine);

			dbmask = *(__m128i*)&_mm_cmpge_ps(W1, dbquad);

			if (_mm_movemask_ps(*(__m128*)&dbmask))
			{
				PreInterpolateVaryings(W1, V1, ps_param.Varyings);

				pPixelShader->Execute(&ps_param);

				nquad = ConvertColorFormat(ps_param.Output);

				// Update Depth Buffer!
				dbquad = _mm_or_ps(_mm_and_ps(W1, *(__m128*)&dbmask), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
				_mm_storeu_ps(depthTileLine, dbquad);

				nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
				_mm_storeu_si128((__m128i*)colorTileLine, nquad);
			}

			colorBuffer += wp;
			depthBuffer += wz;

			IncVaryingsAlongY(W0, W1, WDY, V0, V1, VDY);
		}
	}
}


void Arti3DThread::RenderBlockFragments(Arti3DFragment *i_pFrag)
{
	__m128 W0, W1, WDY;
	__m128 V0[ARTI3D_MAX_VARYING], V1[ARTI3D_MAX_VARYING], VDY[ARTI3D_MAX_VARYING];
	__m128 C0 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
	__m128 C1 = _mm_set_ps1(4.0f);

	PArti3DThread pThread = &m_pParent->m_pThreads[i_pFrag->threadID];
	Arti3DTransformedFace *f = &pThread->m_pTransformedFace[i_pFrag->faceID];

	Arti3DSurface *pbb = m_pParent->m_pRenderTarget->m_pBackbuffer;
	void *pixels = pbb->pGetPixelsDataPtr();
	int wp = pbb->iGetWidth();

	Arti3DSurface *pzb = m_pParent->m_pRenderTarget->m_pZBuffer;
	void *depths = pzb->pGetPixelsDataPtr();
	int wz = pzb->iGetWidth();

	PArti3DPixelShader pPixelShader = m_pParent->mRC.pPixelShader;
	
	float *depthBuffer = (float*)depths + wz * i_pFrag->y + i_pFrag->x;
	uint32_t *colorBuffer = (uint32_t *)pixels + wp * i_pFrag->y + i_pFrag->x;

	for (int y = i_pFrag->y; y < i_pFrag->y + g_ciBlockSize; ++y)
	{
		CalcVaryings(f, i_pFrag->x, y, W0, W1, WDY, V0, V1, VDY);
		SSE_ALIGN Arti3DPSParam ps_param;
		
		__m128	dbquad;
		__m128i dbmask, oquad, nquad;
		uint32_t	*colorTileLine = colorBuffer;
		float		*depthTileLine = depthBuffer;

		dbquad = _mm_loadu_ps(depthTileLine);
		oquad = _mm_loadu_si128((__m128i*)colorTileLine);

		// Depth test
		dbmask = *(__m128i*)&_mm_cmpge_ps(W0, dbquad);

		// If At Least 1 Pixels Passed Test.
		if (_mm_movemask_ps(*(__m128*)&dbmask))
		{
			PreInterpolateVaryings(W0, V0, ps_param.Varyings);
			pPixelShader->Execute(&ps_param);

			nquad = ConvertColorFormat(ps_param.Output);

			// Update Depth Buffer!
			dbquad = _mm_or_ps(_mm_and_ps(W0, *(__m128*)&dbmask), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
			_mm_storeu_ps(depthTileLine, dbquad);

			nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
			_mm_storeu_si128((__m128i*)colorTileLine, nquad);
		}

		colorTileLine += 4;
		depthTileLine += 4;

		dbquad = _mm_loadu_ps(depthTileLine);
		oquad = _mm_loadu_si128((__m128i*)colorTileLine);

		dbmask = *(__m128i*)&_mm_cmpge_ps(W1, dbquad);

		if (_mm_movemask_ps(*(__m128*)&dbmask))
		{
			PreInterpolateVaryings(W1, V1, ps_param.Varyings);

			pPixelShader->Execute(&ps_param);

			nquad = ConvertColorFormat(ps_param.Output);

			// Update Depth Buffer!
			dbquad = _mm_or_ps(_mm_and_ps(W1, *(__m128*)&dbmask), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
			_mm_storeu_ps(depthTileLine, dbquad);

			nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
			_mm_storeu_si128((__m128i*)colorTileLine, nquad);
		}

		colorBuffer += wp;
		depthBuffer += wz;

		IncVaryingsAlongY(W0, W1, WDY, V0, V1, VDY);
	}
}

void Arti3DThread::RenderMaskedFragments(Arti3DFragment *i_pFrag)
{
	__m128 W0, W1, WDY;
	__m128 V0[ARTI3D_MAX_VARYING], V1[ARTI3D_MAX_VARYING], VDY[ARTI3D_MAX_VARYING];
	__m128 C0 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
	__m128 C1 = _mm_set_ps1(4.0f);
	__m128i iMask = _mm_set_epi32(8, 4, 2, 1);

	PArti3DThread pThread = &m_pParent->m_pThreads[i_pFrag->threadID];
	Arti3DTransformedFace *f = &pThread->m_pTransformedFace[i_pFrag->faceID];

	Arti3DSurface *pbb = m_pParent->m_pRenderTarget->m_pBackbuffer;
	void *pixels = pbb->pGetPixelsDataPtr();
	int wp = pbb->iGetWidth();

	Arti3DSurface *pzb = m_pParent->m_pRenderTarget->m_pZBuffer;
	void *depths = pzb->pGetPixelsDataPtr();
	int wz = pzb->iGetWidth();

	PArti3DPixelShader pPixelShader = m_pParent->mRC.pPixelShader;

	float *depthBuffer = (float*)depths + wz * i_pFrag->y + i_pFrag->x;
	uint32_t *colorBuffer = (uint32_t *)pixels + wp * i_pFrag->y + i_pFrag->x;

	CalcVaryings(f, i_pFrag->x, i_pFrag->y, W0, W1, WDY, V0, V1, VDY);

	SSE_ALIGN Arti3DPSParam ps_param;

	__m128	dbquad;
	__m128i cbmask, dbmask, oquad, nquad;
	uint32_t	*colorTileLine = colorBuffer;
	float		*depthTileLine = depthBuffer;

	dbquad = _mm_loadu_ps(depthTileLine);
	oquad = _mm_loadu_si128((__m128i*)colorTileLine);

	cbmask = _mm_cmpgt_epi32(_mm_and_si128(_mm_set1_epi32(i_pFrag->mask), iMask), _mm_setzero_si128());

	// Depth test
	dbmask = *(__m128i*)&_mm_cmpge_ps(W0, dbquad);

	dbmask = _mm_and_si128(dbmask, cbmask);
	// If At Least 1 Pixels Passed Test.
	if (_mm_movemask_ps(*(__m128*)&dbmask))
	{
		PreInterpolateVaryings(W0, V0, ps_param.Varyings);
		pPixelShader->Execute(&ps_param);

		nquad = ConvertColorFormat(ps_param.Output);

		// Update Depth Buffer!
		dbquad = _mm_or_ps(_mm_and_ps(W0, *(__m128*)&dbmask), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
		_mm_storeu_ps(depthTileLine, dbquad);

		nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
		_mm_storeu_si128((__m128i*)colorTileLine, nquad);
	}
}

void Arti3DThread::CalcVaryings(Arti3DTransformedFace* f, int x, int y, __m128 &W0, __m128 &W1, __m128 &WDY, __m128 *V0, __m128 *V1, __m128 *VDY)
{
	float xStep = x - f->v0x;
	float yStep = y - f->v0y;

	// Setup W0 And W1.
	__m128 base = _mm_set_ps1(f->v0w + xStep * f->dw.x + yStep * f->dw.y);
	__m128 dx = _mm_set_ps1(f->dw.x);

	__m128 C1 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
	__m128 C2 = _mm_set_ps1(4.0f);

	W0 = _mm_add_ps(base, _mm_mul_ps(dx, C1));
	W1 = _mm_add_ps(W0, _mm_mul_ps(dx, C2));

	WDY = _mm_set_ps1(f->dw.y);

	for (int i = 0; i < ARTI3D_MAX_VARYING; ++i)
	{
		float dvx = f->dv[i].x;
		float dvy = f->dv[i].y;

		VDY[i] = _mm_set1_ps(dvy);
		
		base = _mm_set_ps1(f->v0v[i] + xStep * dvx + yStep * dvy);
		
		dx = _mm_set_ps1(dvx);

		V0[i] = _mm_add_ps(base, _mm_mul_ps(dx, C1));
		V1[i] = _mm_add_ps(V0[i], _mm_mul_ps(dx, C2));

	}
}

void Arti3DThread::PreInterpolateVaryings(__m128 &W, __m128 *iV, SSE_Float *oV)
{
	__m128 w = _mm_rcp_ps(W);
	for (int i = 0; i < ARTI3D_MAX_VARYING; ++i)
		oV[i] = _mm_mul_ps(w, iV[i]);
}

void Arti3DThread::IncVaryingsAlongY(__m128 &W0, __m128 &W1, __m128 WDY, __m128 *V0, __m128 *V1, __m128 *VDY)
{
	W0 = _mm_add_ps(W0, WDY);
	W1 = _mm_add_ps(W1, WDY);

	for (int i = 0; i < ARTI3D_MAX_VARYING; ++i)
	{
		V0[i] = _mm_add_ps(V0[i], VDY[i]);
		V1[i] = _mm_add_ps(V1[i], VDY[i]);
	}
}

__m128i Arti3DThread::ConvertColorFormat(SSE_Color3 &src)
{
	__m128 fMax = _mm_set_ps1(255.0f);

	src.r = _mm_min_ps(_mm_mul_ps(src.r.f, fMax), fMax);
	src.g = _mm_min_ps(_mm_mul_ps(src.g.f, fMax), fMax);
	src.b = _mm_min_ps(_mm_mul_ps(src.b.f, fMax), fMax);

	__m128i iR = _mm_cvtps_epi32(src.r.f);
	__m128i iG = _mm_cvtps_epi32(src.g.f);
	__m128i iB = _mm_cvtps_epi32(src.b.f);

	iR = _mm_slli_epi32(iR, 16);
	iG = _mm_slli_epi32(iG, 8);

	return _mm_or_si128(_mm_or_si128(iR, iG), iB);
}

