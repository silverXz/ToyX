#include "stdafx.h"
#include <algorithm>
#include "Arti3D_Thread.h"
#include "Arti3D_Device.h"
#include "Arti3D_IndexBuffer.h"
#include "Arti3D_VertexBuffer.h"
#include "Arti3D_VertexLayout.h"
#include "Arti3D_Tile.h"


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

void Arti3DThread::ClearCache()
{
	m_iTransformedFace = 0;

	if (m_pVertexCache)
	{
		for (int i = 0; i < g_ciCacheSize; ++i)
			m_pVertexCache[i].Clear();
	}
}
Arti3DResult Arti3DThread::Create(Arti3DDevice *pParent)
{
	m_pVertexCache = new Arti3DVertexCache[g_ciCacheSize];
	if (!m_pVertexCache)
		return ARTI3D_OUT_OF_MEMORY;

	m_pTransformedFace = new Arti3DTransformedFace[g_ciMaxClipVertexNumPerPatch/(g_ciMaxThreadNum * 3)];
	if (!m_pTransformedFace)
		return ARTI3D_OUT_OF_MEMORY;

	m_pParent = pParent;

	return ARTI3D_OK;
}

unsigned int Arti3DThread::WorkFunc(void *pParam)
{
	Arti3DThread *pThread = (Arti3DThread*)pParam;
	
	return 0;
}

void Arti3DThread::ProcessVertex()
{
	ClearCache();

	for (uint32_t i = m_iStart; i < m_iEnd; i+=3)
	{
		Arti3DVSOutput v[g_ciMaxClipVertex];

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
	uint32_t iCacheIndex = i_iVertexIndex&(g_ciCacheSize - 1);
	if (m_pVertexCache[iCacheIndex].tag == i_iVertexIndex)
	{
		*o_pVSOutput = m_pVertexCache[iCacheIndex].vs_output;
	}
	else
	{
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

		// Try Something new :)

		m_pParent->mRC.pfnVS(&vsinput, &m_pParent->mRC.globals, &m_pVertexCache[iCacheIndex].vs_output);
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

	// No clipping happens,totally inside!
	if (mask == 0x0)
	{
		PostProcessVertex(v1);
		PostProcessVertex(v2);
		PostProcessVertex(v3);
		AddTransformedFace(v1, v2, v3);
		return;
	}

	// Clipping happens! Do the clip work!
	Toy_Plane p[6] = {
		{ -1.0f, 0.0f, 0.0f, 1.0f }, // POS_X_PLANE ( pointing at -x )
		{ 1.0f, 0.0f, 0.0f, 1.0f },	// NEG_X_PLANE ( pointing at +x )
		{ 0.0f, -1.0f, 0.0f, 1.0f },	// POS_Y_PLANE ( pointing at -y )
		{ 0.0f, 1.0f, 0.0f, 1.0 },	// NEG_Y_PLANE ( pointing at +y )
		{ 0.0f, 0.0f, -1.0f, 1.0f },	// POS_Z_PLANE ( pointing at -z )
		{ 0.0f, 0.0f, 1.0f, 0.0f }	// NEG_Z_PLANE ( pointing at +z )
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
	uint32_t inout[2][g_ciMaxClipVertex];
	uint32_t *in = inout[0], *out = inout[1];
	in[0] = 0;	in[1] = 1; in[2] = 2;

	int vCnt = 3;
	int inCnt = 3;
	int outCnt = 0;

	// Several lambda expression to help!

	// Calculate the signed distance between a vertex and a plane.
	auto calcPointPlaneDistance = [](const Toy_Plane *p, const Arti3DVSOutput *v) { 	return p->x * v->p.x + p->y * v->p.y + p->z * v->p.z + p->d * v->p.w; };

	// Determine whether two floats has different signs.
	auto hasDifferentSigns = [](float a, float b) { return (a >= 0.0f && b < 0.0f) || (a < 0.0f && b >= 0.0f);	};

	// Interpolate Vertex Attributes
	auto interpolateV = [](const Arti3DVSOutput *v1, const Arti3DVSOutput *v2, float t, Arti3DVSOutput *out) {
		out->p.x = v1->p.x + (v2->p.x - v1->p.x) * t;
		out->p.y = v1->p.y + (v2->p.y - v1->p.y) * t;
		out->p.z = v1->p.z + (v2->p.z - v1->p.z) * t;
		out->p.w = v1->p.w + (v2->p.w - v1->p.w) * t;
		for (int i = 0; i < g_ciMaxVaryingNum; ++i)
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
	float invW = 1.0f / io_pVSOutput->p.w;
	io_pVSOutput->p.x *= invW;
	io_pVSOutput->p.y *= invW;
	io_pVSOutput->p.w = invW;

	// Transform to screen space.
	io_pVSOutput->p.x = (io_pVSOutput->p.x + 1.0f) * 0.5f * m_pParent->mRT.back_buffer->w;
	io_pVSOutput->p.y = m_pParent->mRT.back_buffer->h - (io_pVSOutput->p.y + 1.0f)*0.5f * m_pParent->mRT.back_buffer->h;

	for (int j = 0; j < g_ciMaxVaryingNum; ++j)
	{
		io_pVSOutput->varyings[j] *= invW;
	}
}

void Arti3DThread::AddTransformedFace(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3)
{
	Arti3DTransformedFace f;

	f.v0x = v1->p.x;
	f.v0y = v1->p.y;
	f.v0w = v1->p.w;

	for (int i = 0; i < g_ciMaxVaryingNum; ++i)
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

	for (int i = 0; i < g_ciMaxVaryingNum; ++i)
	{
		ComputeTriangleGradient(CS, v2->varyings[i] - v1->varyings[i], v3->varyings[i] - v1->varyings[i], fdx21, fdy21, fdx31, fdy31, &f.dv[i]);
	}

	m_pTransformedFace[m_iTransformedFace++] = f;
}

void Arti3DThread::ComputeTriangleGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, toy::vec2 *o_pVec2)
{
	float A = di21 * dy31 - di31 * dy21;
	float B = di21 * dx31 - di31 * dx21;
	o_pVec2->x = A / C;
	o_pVec2->y = -B / C;
}

void Arti3DThread::PreProcessTile()
{
	Arti3DTile *pTile = m_pParent->m_pTiles;

	uint32_t iTileX = m_pParent->m_iTileX;
	uint32_t iTileY = m_pParent->m_iTileY;

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


		int xmin = std::min(std::min(f->fp1[0], f->fp2[0]), f->fp3[0]);
		int xmax = std::max(std::max(f->fp1[0], f->fp2[0]), f->fp3[0]);
		int ymin = std::min(std::min(f->fp1[1], f->fp2[1]), f->fp3[1]);
		int ymax = std::max(std::max(f->fp1[1], f->fp2[1]), f->fp3[1]);

		// Boundary Index For Tiles
		int ixMinTile = ((xmin + 0xF) >> 4) >> g_ciTileSizeShift;
		int ixMaxTile = ((xmax + 0xF) >> 4) >> g_ciTileSizeShift;
		int iyMinTile = ((ymin + 0xF) >> 4) >> g_ciTileSizeShift;
		int iyMaxTile = ((ymax + 0xF) >> 4) >> g_ciTileSizeShift;

		// Tile[x][y]
		ixMaxTile = ixMinTile >= iTileX ? iTileX - 1 : ixMaxTile;
		iyMaxTile = iyMaxTile >= iTileY ? iTileY - 1 : iyMaxTile;


		for (int y = iyMinTile; y <= iyMaxTile; ++y)
		{
			for (int x = ixMinTile; x <= ixMaxTile; ++x)
			{
				// Get Tile Corner Fix Point Coordinate.
				int iTile = y * iTileX + x;

				Arti3DTile &tile = pTile[iTile];

				int x0 = tile.m_iX << 4;
				int x1 = (tile.m_iX + g_ciTileSize - 1) << 4;
				int y0 = tile.m_iY << 4;
				int y1 = (tile.m_iY + g_ciTileSize - 1) << 4;

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
			}
		}
	}
}
