#include "stdafx.h"
#include "Arti3D_Device.h"
#include <algorithm>
#include <cstdlib>
#include <assert.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include "Arti3D_VertexLayout.h"
#include "Arti3D_VertexBuffer.h"
#include "Arti3D_IndexBuffer.h"
#include "Arti3D_Thread.h"
#include "Arti3D_Tile.h"


using namespace a3d;

Arti3DDevice::Arti3DDevice() : m_pIndexBuffer(nullptr),
	m_pVertexBuffer(nullptr),
	m_pVertexLayout(nullptr),
	m_pThreads(nullptr),
	m_pTiles(nullptr),
	m_pJobQueue(nullptr),
	m_iJobStart(0),
	m_iJobEnd(0),
	m_iJobStart2(0),
	m_iWorkingThread(0),
	m_iStage(1),
	m_bThreadStop(false)
{
	dFile.open("Debug.txt", std::ios::ate);
}

Arti3DDevice::~Arti3DDevice()
{
	dFile.close();

	StopAllThreads();

	SAFE_DELETE(m_pIndexBuffer);
	SAFE_DELETE(m_pVertexBuffer);
	SAFE_DELETE(m_pVertexLayout);
	SAFE_DELETE_ARRAY(m_pThreads);
	SAFE_DELETE_ARRAY(m_pTiles);
	SAFE_DELETE_ARRAY(m_pJobQueue);
}



void Arti3DDevice::ClearColorBuffer(const ToyColor& color)
{
	int hr = SDL_FillRect(mRT.back_buffer, nullptr, color.ToUInt32());
	if (hr)
		std::cerr << "Failed to clear color buffer!\n";
}

void Arti3DDevice::ClearDepthBuffer(float cDepth)
{
	SDL_FillRect(mRT.z_buffer, nullptr, 0);
}

void Arti3DDevice::SetMatrix(Arti3DMatrixType matrixType, const a3d::mat4& m)
{
	Arti3DShaderUniform &rgu = mRC.globals;
	switch (matrixType)
	{
	case TOY_MATRIX_MODEL:
		rgu.model = m;
		break;
	case TOY_MATRIX_VIEW:
		rgu.view = m;
		break;
	case TOY_MATRIX_PROJECTION:
		rgu.projection = m;
		break;
	default:
		break;
	}
	rgu.mvp = rgu.projection * rgu.view * rgu.model;
}

void Arti3DDevice::SetViewport(int x, int y, int width, int height)
{
	mRC.globals.viewport = vec4(FLOAT_CAST(x),FLOAT_CAST(y), FLOAT_CAST(width), FLOAT_CAST(height));
}

void Arti3DDevice::Draw2DLines(int x1, int y1, int x2, int y2, uint32_t color)
{
	int dy = abs(y2 - y1);
	int dx = abs(x2 - x1);
	if (dx == 0 && dy == 0)
		SetPixelColor(x1, y1, color);
	int signx = x1 < x2 ? 1 : -1;
	int signy = y1 < y2 ? 1 : -1;
	bool bSwap = false;
	if (dy > dx)
	{
		std::swap(dy, dx);
		bSwap = true;
	}

	int err = 2 * dy - dx;

	int curx = x1;
	int cury = y1;

	for (int i = 0; i <= dx; ++i)
	{
		SetPixelColor(curx, cury, color);
		if (err > 0)
		{
			if (bSwap)
				curx += signx;
			else
				cury += signy;
			err -= 2 * dx;
		}

		if (bSwap)
			cury += signy;
		else
			curx += signx;

		err += 2 * dy;
	}
}

void Arti3DDevice::Draw3DLines(const a3d::vec4& p1, const a3d::vec4 p2, uint32_t color)
{
	vec4 clip1 = mRC.globals.mvp * p1;
	vec4 clip2 = mRC.globals.mvp * p2;
	// perspective division
	float invW1 = 1.0f / clip1.w;
	clip1.x *= invW1;
	clip1.y *= invW1;
	clip1.z *= invW1;

	float invW2 = 1.0f / clip2.w;
	clip2.x *= invW2;
	clip2.y *= invW2;
	clip2.z *= invW2;
	Draw2DLines(iRound(clip1.x), iRound(clip1.y), iRound(clip2.x), iRound(clip2.y), color);
}

void Arti3DDevice::Draw3DSolidTriangle(const a3d::vec4& p1, const a3d::vec4& p2, const a3d::vec4& p3, const ToyColor& c)
{
	vec4 clip1 = mRC.globals.mvp * p1;
	vec4 clip2 = mRC.globals.mvp * p2;
	vec4 clip3 = mRC.globals.mvp * p3;
	
	float invW1 = 1.0f / clip1.w;
	clip1.x *= invW1;
	clip1.y *= invW1;

	float invW2 = 1.0f / clip2.w;
	clip2.x *= invW2;
	clip2.y *= invW2;

	float invW3 = 1.0f / clip3.w;
	clip3.x *= invW3;
	clip3.y *= invW3;

	float half_width = 0.5f * mRT.back_buffer->w;
	float half_height = 0.5f * mRT.back_buffer->h;

	clip1.x = (clip1.x + 1.0f) * half_width;
	clip1.y = mRT.back_buffer->h - (clip1.y + 1.0f) * half_height;
	clip2.x = (clip2.x + 1.0f) * half_width;
	clip2.y = mRT.back_buffer->h - (clip2.y + 1.0f) * half_height;
	clip3.x = (clip3.x + 1.0f) * half_width;
	clip3.y = mRT.back_buffer->h - (clip3.y + 1.0f) * half_height;	
}

void Arti3DDevice::LoadCube()
{
	const float len = 2.0f;

	Arti3DVertexLayout *pVertexLayout = nullptr;
	Arti3DVertexAttributeFormat vaf[] = { ARTI3D_VAF_VECTOR4, ARTI3D_VAF_VECTOR4 };
	CreateVertexLayout(&pVertexLayout, 2, vaf);
	SetVertexLayout(pVertexLayout);

	Arti3DVertexBuffer *pVertexBuffer = nullptr;
	
	uint32_t iFloat = pVertexLayout->iGetFloats();
	uint32_t iStride = iFloat * sizeof(float);
	const uint32_t iVertex = 8;
	CreateVertexBuffer(&pVertexBuffer, iVertex * iStride);

	// Upload Cube Data To VertexBuffer
	std::vector<std::vector<float>> xv{
		{ -len, len, len, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
		{ len, len, len, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
		{ len, len, -len, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f },
		{ -len, len, -len, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f },
		{ -len, -len, len, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f },
		{ len, -len, len, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ len, -len, -len, 1.0f,1.0f, 1.0f, 1.0f, 0.0f },
		{ -len, -len, -len, 1.0f,0.0f, 0.0f, 0.0f, 1.0f }
	};

	for (int i = 0; i < iVertex; ++i)
	{
		void *pDest = nullptr;
		pVertexBuffer->GetPointer(i * iStride, &pDest);
		memcpy(pDest, &xv[i][0], iStride);
	}

	SetVertexBuffer(pVertexBuffer);

	Arti3DIndexBuffer *pIndexBuffer = nullptr;
	CreateIndexBuffer(&pIndexBuffer, 36 * sizeof(uint32_t), ARTI3D_INDEX32);
	uint32_t xid[] = { 0, 1, 2, 0, 2, 3, 0, 4, 5, 0, 5, 1, 1, 5, 6, 1, 6, 2, 4, 7, 6, 4, 6, 5, 0, 3, 7, 0, 7, 4, 3, 2, 6, 3, 6, 7 };
	void *pDest = nullptr;
	pIndexBuffer->GetPointer(0, &pDest);
	memcpy(pDest, xid, sizeof(xid));

	SetIndexBuffer(pIndexBuffer);
}

void Arti3DDevice::Begin()
{
	if (!mRT.back_buffer)
	{
		std::cerr << "ToyRender::Begin(): No Backbuffer!\n";
		return;
	}
	SDL_LockSurface(mRT.back_buffer);

}

void Arti3DDevice::End()
{
	SDL_UnlockSurface(mRT.back_buffer);
}

void Arti3DDevice::SetRenderTarget(const RenderTarget &rRT)
{
	mRT = rRT;
}


void Arti3DDevice::GetTransformedVertex(uint32_t i_iVertexIndex, Arti3DVSOutput *out)
{
	uint32_t iCacheIndex = i_iVertexIndex&(g_ciCacheSize - 1);
	if (m_VSOutputCache[iCacheIndex].tag == i_iVertexIndex)
	{
		*out = m_VSOutputCache[iCacheIndex].vs_output;
	}
	else
	{
		Arti3DVSInput vsinput;

		void *pSrc = nullptr;
		m_pVertexBuffer->GetPointer(m_pVertexLayout->iGetFloats() * sizeof(float) * i_iVertexIndex, &pSrc);
		float *pV = (float*)pSrc;
		uint32_t iAttributeNum = 0;
		m_pVertexLayout->iGetAttributeNum(&iAttributeNum);
		for (uint32_t i = 0; i < iAttributeNum; ++i)
		{
			switch (m_pVertexLayout->m_pVertexAttributeFormat[i])
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

 		mRC.pfnVS(&vsinput, &mRC.globals, &m_VSOutputCache[iCacheIndex].vs_output);
 		m_VSOutputCache[iCacheIndex].tag = i_iVertexIndex;
 		*out = m_VSOutputCache[iCacheIndex].vs_output;
	}
}

void Arti3DDevice::ProcessV()
{
	faceBuffer.clear();
	ClearCache();

	uint32_t iIndexNum = m_pIndexBuffer->iGetIndexNum();

	for (uint32_t i = 0; i < iIndexNum; i += 3)
	{
		Arti3DVSOutput v[g_ciMaxClipVertex];

		for (int j = 0; j < 3; ++j)
		{
			uint32_t iVertexIndex = 0;
			m_pIndexBuffer->GetVertexIndex(i + j, &iVertexIndex);
			GetTransformedVertex(iVertexIndex, &v[j]);
		}
		InsertTransformedFace(&v[0], &v[1], &v[2]);
	}

}

void Arti3DDevice::ProcessV_WithClip()
{
	faceBuffer.clear();
	ClearCache();

	uint32_t iIndexNum = m_pIndexBuffer->iGetIndexNum();

	for (uint32_t i = 0; i < iIndexNum; i += 3)
	{
		Arti3DVSOutput v[g_ciMaxClipVertex];

		for (int j = 0; j < 3; ++j)
		{
			uint32_t iVertexIndex = 0;
			m_pIndexBuffer->GetVertexIndex(i + j, &iVertexIndex);
			GetTransformedVertex(iVertexIndex, &v[j]);
		}
		ClipTriangle(&v[0], &v[1], &v[2]);
	}

}

void Arti3DDevice::ClipTriangle(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3)
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
		PostProcessV(v1);
		PostProcessV(v2);
		PostProcessV(v3);
		InsertTransformedFace(v1, v2, v3);
		return;
	}

	// Clipping happens! Do the clip work!
	Toy_Plane p[6] = {
		{-1.0f,0.0f,0.0f,1.0f}, // POS_X_PLANE ( pointing at -x )
		{1.0f,0.0f,0.0f,1.0f},	// NEG_X_PLANE ( pointing at +x )
		{0.0f,-1.0f,0.0f,1.0f},	// POS_Y_PLANE ( pointing at -y )
		{0.0f,1.0f,0.0f,1.0},	// NEG_Y_PLANE ( pointing at +y )
		{0.0f,0.0f,-1.0f,1.0f},	// POS_Z_PLANE ( pointing at -z )
		{0.0f,0.0f,1.0f,0.0f}	// NEG_Z_PLANE ( pointing at +z )
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
	auto calcPointPlaneDistance = [](const Toy_Plane *p, const Arti3DVSOutput *v) { 	return p->x * v->p.x + p->y * v->p.y + p->z * v->p.z + p->d * v->p.w;};

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
			float d1 = calcPointPlaneDistance(&p[i],&v[id1]);
			
			for (int j = 1; j <= inCnt; ++j)
			{
				int id2 = in[j];
				float d2 = calcPointPlaneDistance(&p[i],&v[id2]);

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
		PostProcessV(p);
	}

	for (int i = 1; i < inCnt - 1; ++i)
		InsertTransformedFace(&v[in[0]], &v[in[i]], &v[in[i + 1]]);
}

void Arti3DDevice::InsertTransformedFace(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3)
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
	ComputeGradient(CS, v2->p.w - v1->p.w, v3->p.w - v1->p.w, fdx21, fdy21, fdx31, fdy31, &f.dw);

	for (int i = 0; i < g_ciMaxVaryingNum; ++i)
	{
		ComputeGradient(CS, v2->varyings[i] - v1->varyings[i], v3->varyings[i] - v1->varyings[i], fdx21, fdy21, fdx31, fdy31, &f.dv[i]);
	}

	faceBuffer.push_back(f);
}

int Arti3DDevice::CalcClipMask(Arti3DVSOutput *v)
{
	int mask = 0;
	if (v->p.x - v->p.w > 0) mask |= CLIP_POS_X;
	if (v->p.x + v->p.w < 0) mask |= CLIP_NEG_X;
	if (v->p.y - v->p.w > 0) mask |= CLIP_POS_Y;
	if (v->p.y + v->p.w < 0) mask |= CLIP_NEG_Y;
	if (v->p.z - v->p.w > 0) mask |= CLIP_POS_Z;
	if (v->p.z + v->p.w < 0) mask |= CLIP_NEG_Z;
	return mask;
}

void Arti3DDevice::ProcessR()
{
	for (auto &i : faceBuffer)
	{
		RasterizeTriangle_SIMD(&i);
	}
}

void Arti3DDevice::PostProcessV(Arti3DVSOutput *v)
{
	float invW = 1.0f / v->p.w;
	v->p.x *= invW;
	v->p.y *= invW;
	v->p.w = invW;

	// Transform to screen space.
	v->p.x = (v->p.x + 1.0f) * 0.5f * mRT.back_buffer->w;
	v->p.y = mRT.back_buffer->h - (v->p.y + 1.0f)*0.5f *mRT.back_buffer->h;

	for (int j = 0; j < g_ciMaxVaryingNum; ++j)
	{
		v->varyings[j] *= invW;
	}
}

void Arti3DDevice::DrawMesh()
{
	ProcessV_WithClip();
	
	ProcessR();

#ifdef _DEBUG
	DrawTileGrid();
#endif

}


void Arti3DDevice::DrawMesh_TileBase()
{
	ProcessV_WithClip();

	ClearTile();
	for (uint32_t i = 0; i < faceBuffer.size(); ++i)
		Tilize(i);

	RasterizeTile();

	RenderFragments();

#ifdef _DEBUG
	DrawTileGrid();
#endif
}


void Arti3DDevice::ComputeGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, a3d::vec2 *g)
{
	float A = di21 * dy31 - di31 * dy21;
	float B = di21 * dx31 - di31 * dx21;
	g->x = A / C;
	g->y = -B / C;
}

void Arti3DDevice::ClearCache()
{
	for (auto i = 0; i < g_ciCacheSize; ++i)
	{
		m_VSOutputCache[i].Clear();
	}
}


void Arti3DDevice::RasterizeTriangle_SIMD(Arti3DTransformedFace *f)
{
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

	const int blockSize = 8;

	int xmin = std::min(std::min(f->fp1[0], f->fp2[0]), f->fp3[0]);
	int xmax = std::max(std::max(f->fp1[0], f->fp2[0]), f->fp3[0]);
	int ymin = std::min(std::min(f->fp1[1], f->fp2[1]), f->fp3[1]);
	int ymax = std::max(std::max(f->fp1[1], f->fp2[1]), f->fp3[1]);

	int cxMin = ((xmin + 0xF) >> 4) & (~(blockSize - 1));
	int cxMax = (((xmax + 0xF) >> 4) + blockSize) &(~(blockSize - 1));
	int cyMin = ((ymin + 0xF) >> 4) &(~(blockSize - 1));
	int cyMax = (((ymax + 0xF) >> 4) + blockSize) &(~(blockSize - 1));

	cxMax = cxMax > mRT.back_buffer->w ? mRT.back_buffer->w : cxMax;
	cyMax = cyMax > mRT.back_buffer->h ? mRT.back_buffer->h : cyMax;
	

	int E1 = DY21 * (cxMin << 4) - DX21 * (cyMin << 4) + C1;
	int E2 = DY32 * (cxMin << 4) - DX32 * (cyMin << 4) + C2;
	int E3 = DY13 * (cxMin << 4) - DX13 * (cyMin << 4) + C3;

	for (int y = cyMin; y < cyMax; y += blockSize)
	{
		for (int x = cxMin; x < cxMax; x += blockSize)
		{
			// Test Block Corners!
			int x0 = x << 4;
			int x1 = (x + blockSize - 1) << 4;
			int y0 = y << 4;
			int y1 = (y + blockSize - 1) << 4;

			auto calcEdgeMask = [&](int C, int dy, int dx) {
				bool m0 = (C + dy * x0 - dx * y0) > 0;
				bool m1 = (C + dy * x0 - dx * y1) > 0;
				bool m2 = (C + dy * x1 - dx * y0) > 0;
				bool m3 = (C + dy * x1 - dx * y1) > 0;
				return (m0 << 0) | (m1 << 1) | (m2 << 2) | (m3 << 3);
			};

			// Test Block Against 3 Edges
			int a1 = calcEdgeMask(C1, DY21, DX21);
			int a2 = calcEdgeMask(C2, DY32, DX32);
			int a3 = calcEdgeMask(C3, DY13, DX13);

			// Complete Outside Triangle!
			if (a1 == 0 || a2 == 0 || a3 == 0)
				continue;

			float *depthBuffer = mRT.z_buffer ? (float*)mRT.z_buffer->pixels + y * mRT.z_buffer->w + x : nullptr;
			uint32_t *colorBuffer = (uint32_t*)mRT.back_buffer->pixels + y * mRT.back_buffer->w + x;
			
			// Totally Inside, Handle blockSize * blockSize PIxels
			if (a1 == 0xF && a2 == 0xF && a3 == 0xF)
			{
				__m128 W0, W1, WDY;
				__m128 V0[g_ciMaxVaryingNum], V1[g_ciMaxVaryingNum], VDY[g_ciMaxVaryingNum];
				__m128 C0 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
				__m128 C1 = _mm_set_ps1(4.0f);
				__m128 fMax = _mm_set_ps1(255.0f);

				

				CalcVaryings(f, x, y, W0, W1, WDY, V0, V1, VDY);

				// Process blockSize pixels every time, do it blockSize times.
				for (int iy = 0; iy < blockSize; iy++)
				{
					SSE_ALIGN Arti3DPSParam parm;

					__m128 dbquad;
					__m128i oquad, nquad, dbmask;
					uint32_t *cbTileLine;
					float *dbTileLine = nullptr;

					dbTileLine = depthBuffer;
					dbquad = _mm_loadu_ps(dbTileLine);

					cbTileLine = colorBuffer;

					oquad = _mm_loadu_si128((__m128i*)cbTileLine);

					// Depth Comparison.
					dbmask = *(__m128i*)&_mm_cmpge_ps(W0, dbquad);

					// Not Zero : means at least 1 pixel passed the depth test!
					if (_mm_movemask_ps(*(__m128*)&dbmask))
					{
						PreInterpolateVaryings(W0, V0, parm.Varyings);


						mRC.pfnPS(&parm);

						nquad = ConvertColorFormat(parm.Output);

						// Store Depth Info.
						dbquad = _mm_or_ps(_mm_and_ps(*(__m128*)&dbmask, W0), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
						_mm_storeu_ps(dbTileLine, dbquad);
						
						// Store Color Info.
						nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
						_mm_storeu_si128((__m128i*)cbTileLine, nquad);
						
					}

					// Deal with the next 4 pixels
					dbTileLine = depthBuffer + 4;
					dbquad = _mm_loadu_ps(dbTileLine);

					cbTileLine = colorBuffer + 4;
					oquad = _mm_loadu_si128((__m128i*)cbTileLine);


					dbmask = *(__m128i*)&_mm_cmpge_ps(W1, dbquad);
					// At least one pixel passed depth test!
					if (_mm_movemask_ps(*(__m128*)&dbmask))
					{
						PreInterpolateVaryings(W1, V1, parm.Varyings);

						mRC.pfnPS(&parm);

						nquad = ConvertColorFormat(parm.Output);

						// Store Results

						// Store Depth Info.
						dbquad = _mm_or_ps(_mm_and_ps(*(__m128*)&dbmask, W0), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
						_mm_storeu_ps(dbTileLine, dbquad);
						
						// Store Color Info.
						nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
						_mm_storeu_si128((__m128i*)cbTileLine, nquad);
					}

					// Util now, this blockSize pixels has been dealt with. Get Ready for the next 8.

					IncVaryingsAlongY(W0, W1, WDY, V0, V1, VDY);

					colorBuffer += mRT.back_buffer->w;
					depthBuffer += mRT.z_buffer->w;
				}
				continue;
			}


			// The blockSize * blockSize Tile Is Partially Covered By The Triangle.
			__m128i	EB1 = _mm_set1_epi32(DY21 * x0 - DX21 * y0 + C1);
			__m128i EB2 = _mm_set1_epi32(DY32 * x0 - DX32 * y0 + C2);
			__m128i EB3 = _mm_set1_epi32(DY13 * x0 - DX13 * y0 + C3);

			__m128 offset1 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
			__m128 offset2 = _mm_set1_ps(4.0f);

			__m128i offsetDY21 = _mm_set_epi32(DDY21 * 3, DDY21 * 2, DDY21, 0);
			__m128i offsetDY32 = _mm_set_epi32(DDY32 * 3, DDY32 * 2, DDY32, 0);
			__m128i offsetDY13 = _mm_set_epi32(DDY13 * 3, DDY13 * 2, DDY13, 0);
			
			__m128i offsetDY21ex = _mm_set1_epi32(DDY21 * 4);
			__m128i offsetDY32ex = _mm_set1_epi32(DDY32 * 4);
			__m128i offsetDY13ex = _mm_set1_epi32(DDY13 * 4);

			__m128 W0, W1, WDY;
			__m128 V0[g_ciMaxVaryingNum], V1[g_ciMaxVaryingNum], VDY[g_ciMaxVaryingNum];

			CalcVaryings(f, x, y, W0, W1, WDY, V0, V1, VDY);

			for (int i = y; i < y + blockSize; ++i)
			{
				// Calculate Coverage Mask For 4 Pixels...
				__m128i Edg1 = _mm_add_epi32(EB1, offsetDY21);
				__m128i msk1 = _mm_cmpgt_epi32(Edg1, _mm_setzero_si128());
				
				__m128i Edg2 = _mm_add_epi32(EB2, offsetDY32);
				__m128i msk2 = _mm_cmpgt_epi32(Edg2, _mm_setzero_si128());
				
				__m128i Edg3 = _mm_add_epi32(EB3, offsetDY13);
				__m128i msk3 = _mm_cmpgt_epi32(Edg3, _mm_setzero_si128());
				
				__m128i cMask = _mm_and_si128(_mm_and_si128(msk1, msk2), msk3);
			
				SSE_ALIGN Arti3DPSParam parm;

				__m128 dbquad;
				__m128i oquad, nquad, dbmask;
				uint32_t *cbTileLine = nullptr;
				float *dbTileLine = nullptr;

				dbTileLine = depthBuffer;
				dbquad = _mm_loadu_ps(dbTileLine);

				cbTileLine = colorBuffer;

				oquad = _mm_loadu_si128((__m128i*)cbTileLine);

				// Depth Comparison.
				dbmask = *(__m128i*)&_mm_cmpge_ps(W0, dbquad);
				cMask = _mm_and_si128(dbmask, cMask);

				// Not Zero : means at least 1 pixel passed the depth test!
				if (_mm_movemask_ps(*(__m128*)&cMask))
				{
					__m128 w = _mm_rcp_ps(W0);
					for (int f = 0; f < g_ciMaxVaryingNum; f += 2)
					{
						parm.Varyings[f + 0] = _mm_mul_ps(w, V0[f + 0]);
						parm.Varyings[f + 1] = _mm_mul_ps(w, V0[f + 1]);
					}

					mRC.pfnPS(&parm);

					nquad = ConvertColorFormat(parm.Output);

					// Store Depth Info.
					dbquad = _mm_or_ps(_mm_and_ps(*(__m128*)&cMask, W0), _mm_andnot_ps(*(__m128*)&cMask, dbquad));
					_mm_storeu_si128((__m128i*)dbTileLine, *(__m128i*)&dbquad);
											// Store Color Info.
					nquad = _mm_or_si128(_mm_and_si128(cMask, nquad), _mm_andnot_si128(cMask, oquad));
					_mm_storeu_si128((__m128i*)cbTileLine, nquad);

				}

				// Calculate Coverage Mask For The Next 4 Pixels;
				Edg1 = _mm_add_epi32(Edg1, offsetDY21ex);
				msk1 = _mm_cmpgt_epi32(Edg1, _mm_setzero_si128());

				Edg2 = _mm_add_epi32(Edg2, offsetDY32ex);
				msk2 = _mm_cmpgt_epi32(Edg2, _mm_setzero_si128());

				Edg3 = _mm_add_epi32(Edg3, offsetDY13ex);
				msk3 = _mm_cmpgt_epi32(Edg3, _mm_setzero_si128());

				cMask = _mm_and_si128(_mm_and_si128(msk1, msk2), msk3);
				
				dbTileLine = depthBuffer + 4;
				cbTileLine = colorBuffer + 4;
				dbquad = _mm_loadu_ps(dbTileLine);
				oquad = _mm_loadu_si128((__m128i*)cbTileLine);

				dbmask = *(__m128i*)&_mm_cmpge_ps(W1, dbquad);

				cMask = _mm_and_si128(cMask, dbmask);
				// cMask != 0 Means : At Least 1 Pixels Is Covered
				if (_mm_movemask_ps(*(__m128*)&cMask))
				{
					// Calculate The 4 Pixels' Color And Update The Back Buffer According To The "cMask".
					__m128 w = _mm_rcp_ps(W1);
					for (int f = 0; f < g_ciMaxVaryingNum; f += 2)
					{
						parm.Varyings[f + 0] = _mm_mul_ps(w, V1[f + 0]);
						parm.Varyings[f + 1] = _mm_mul_ps(w, V1[f + 1]);
					}

					mRC.pfnPS(&parm);

					nquad = ConvertColorFormat(parm.Output);

					// Store Depth Info.
					dbquad = _mm_or_ps(_mm_and_ps(*(__m128*)&cMask, W1), _mm_andnot_ps(*(__m128*)&cMask, dbquad));
					_mm_storeu_si128((__m128i*)dbTileLine, *(__m128i*)&dbquad);

					// Store Color Info.
					nquad = _mm_or_si128(_mm_and_si128(cMask, nquad), _mm_andnot_si128(cMask, oquad));
					_mm_storeu_si128((__m128i*)cbTileLine, nquad);
				}

				// Setup For The Next 8 Pixels In The Next Row

				IncVaryingsAlongY(W0, W1, WDY, V0, V1, VDY);
				
				colorBuffer += mRT.back_buffer->w;
				depthBuffer += mRT.z_buffer->w;
				
				EB1 = _mm_sub_epi32(EB1, _mm_set1_epi32(DDX21));
				EB2 = _mm_sub_epi32(EB2, _mm_set1_epi32(DDX32));
				EB3 = _mm_sub_epi32(EB3, _mm_set1_epi32(DDX13));
			}
		}
	}
}

void Arti3DDevice::CalcVaryings(Arti3DTransformedFace* f, int x, int y, __m128 &W0, __m128 &W1, __m128 &WDY, __m128 *V0, __m128 *V1, __m128 *VDY)
{
	float xStep = x - f->v0x;
	float yStep = y - f->v0y;
	
	// Setup W0 And W1.
	__m128 base = _mm_set_ps1(f->v0w + xStep * f->dw.x + yStep * f->dw.y);
	__m128 dx = _mm_set_ps1(f->dw.x);

	__m128 C1 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
	__m128 C2 = _mm_set_ps1(4.0f);

	W0 = _mm_add_ps(base, _mm_mul_ps(dx,C1));
	W1 = _mm_add_ps(W0, _mm_mul_ps(dx,C2));

	WDY = _mm_set_ps1(f->dw.y);

	for (int i = 0; i < g_ciMaxVaryingNum; ++i)
	{
		base = _mm_set_ps1(f->v0v[i] + xStep * f->dv[i].x + yStep * f->dv[i].y);
		dx = _mm_set_ps1(f->dv[i].x);

		V0[i] = _mm_add_ps(base, _mm_mul_ps(dx, C1));
		V1[i] = _mm_add_ps(V0[i], _mm_mul_ps(dx, C2));
		VDY[i] = _mm_set1_ps(f->dv[i].y);
	}
}

void Arti3DDevice::IncVaryingsAlongY(__m128 &W0, __m128 &W1, __m128 WDY, __m128 *V0, __m128 *V1, __m128 *VDY)
{
	W0 = _mm_add_ps(W0, WDY);
	W1 = _mm_add_ps(W1, WDY);

	for (int i = 0; i < g_ciMaxVaryingNum; ++i)
	{
		V0[i] = _mm_add_ps(V0[i], VDY[i]);
		V1[i] = _mm_add_ps(V1[i], VDY[i]);
	}
}

__m128i Arti3DDevice::ConvertColorFormat(SSE_Color3 &src)
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

void Arti3DDevice::InitTile()
{

	m_iTileX = (m_iWidth + g_ciTileSize - 1) >> g_ciTileSizeShift;
	m_iTileY = (m_iHeight + g_ciTileSize - 1) >> g_ciTileSizeShift;

	m_aTile.resize(m_iTileX * m_iTileY);

	for (int y = 0; y < m_iTileY; ++y)
	{
		for (int x = 0; x < m_iTileX; ++x)
		{
			Arti3_DTile &tile = m_aTile[y * m_iTileX + x];
			tile.x = x * g_ciTileSize;
			tile.y = y * g_ciTileSize;
			if (x == m_iTileX - 1)
			{
				int iExtraX = m_iWidth & (g_ciTileSize - 1);
				tile.w = iExtraX ? iExtraX : g_ciTileSize;
			}
			else
				tile.w = g_ciTileSize;

			if (y == m_iTileY - 1)
			{
				int iExtraY = m_iHeight & (g_ciTileSize - 1);
				tile.h = iExtraY ? iExtraY : g_ciTileSize;
			}
			else
				tile.h = g_ciTileSize;
		}
	}
}

void Arti3DDevice::Tilize(uint32_t faceid)
{
	Arti3DTransformedFace *f = &faceBuffer[faceid];
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
	ixMaxTile = ixMinTile >= m_iTileX ? m_iTileX - 1 : ixMaxTile;
	iyMaxTile = iyMaxTile >= m_iTileY ? m_iTileY - 1 : iyMaxTile;


	for (int y = iyMinTile; y <= iyMaxTile; ++y)
	{
		for (int x = ixMinTile; x <= ixMaxTile; ++x)
		{
			// Get Tile Corner Fix Point Coordinate.
			int iTile = y * m_iTileX + x;

			Arti3_DTile &tile = m_aTile[iTile];

			int x0 = tile.x << 4;
			int x1 = (tile.x + g_ciTileSize - 1) << 4;
			int y0 = tile.y << 4;
			int y1 = (tile.y + g_ciTileSize - 1) << 4;

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
				if(tile.w == g_ciTileSize && tile.h == g_ciTileSize)
					tile.aTilizedFace.push_back(Arti3D_TiledFace{ faceid, ARTI3D_TC_ALL });
			}
			else
				tile.aTilizedFace.push_back(Arti3D_TiledFace{ faceid, ARTI3D_TC_PARTIAL });
		}
	}
}

void Arti3DDevice::ClearTile()
{
	for (auto& x : m_aTile)
	{
		x.aTilizedFace.clear();
		x.aFragment.clear();
	}
}

void Arti3DDevice::RasterizeTile()
{
	for (size_t i = 0; i < m_aTile.size(); ++i)
	{
		Arti3_DTile &tile = m_aTile[i];
		for (size_t j = 0; j < tile.aTilizedFace.size(); ++j)
		{
			Arti3D_TiledFace &tf = tile.aTilizedFace[j];

			// This Face is totally covering this tile.
			if (tf.coverageType == ARTI3D_TC_ALL)
			{
				Arti3DFragment frag;
				frag.x = tile.x;
				frag.y = tile.y;
				frag.coverType = ARTI3D_FC_TILE;
				frag.faceID = tf.id;
				tile.aFragment.push_back(frag);
				continue;
			}
			
			// This face is partially covering this tile.
			Arti3DTransformedFace *f = &faceBuffer[tf.id];

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

			const int blockSize = 8;

			// For Every 8 * 8 Blocks.
			for (int y = tile.y; y < tile.y + tile.h; y += blockSize)
			{
				for (int x = tile.x; x < tile.x + tile.w; x += blockSize)
				{
					int x0 = x << 4;
					int x1 = ( x + blockSize - 1 ) << 4;
					int y0 = y << 4;
					int y1 = ( y + blockSize - 1 ) << 4;

					auto calcEdgeMask = [&](int C, int dy, int dx) {
						bool m0 = (C + dy * x0 - dx * y0) > 0;
						bool m1 = (C + dy * x0 - dx * y1) > 0;
						bool m2 = (C + dy * x1 - dx * y0) > 0;
						bool m3 = (C + dy * x1 - dx * y1) > 0;
						return (m0 << 0) | (m1 << 1) | (m2 << 2) | (m3 << 3);
					};

					// Test Block Against 3 Edges
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
						frag.x = x;
						frag.y = y;
						frag.coverType = ARTI3D_FC_BLOCK;
						frag.faceID = tf.id;
						tile.aFragment.push_back(frag);
						continue;
					}

					// Block Partially Inside Trianglel!
					// Calculate Coverage Mask!
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

					
					for (int k = 0; k < blockSize; ++k)
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
							frag.x = x;
							frag.y = y + k;
							frag.faceID = tf.id;
							frag.mask = im;
							frag.coverType = ARTI3D_FC_MASKED;
							tile.aFragment.push_back(frag);
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
							frag.x = x + 4;
							frag.y = y + k;
							frag.faceID = tf.id;
							frag.mask = im;
							frag.coverType = ARTI3D_FC_MASKED;
							tile.aFragment.push_back(frag);
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

void Arti3DDevice::RenderFragments()
{
	for (auto& tile : m_aTile)
	{
		for (auto& frag : tile.aFragment)
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
}

void Arti3DDevice::RenderTileFragments(Arti3DFragment *frag)
{
	__m128 W0, W1, WDY;
	__m128 V0[g_ciMaxVaryingNum], V1[g_ciMaxVaryingNum], VDY[g_ciMaxVaryingNum];
	__m128 C0 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
	__m128 C1 = _mm_set_ps1(4.0f);

	Arti3DTransformedFace *f = &faceBuffer[frag->faceID];
	
	float *depthBuffer = nullptr;
	uint32_t *colorBuffer = nullptr;
	
	for (int x = frag->x; x < frag->x + g_ciTileSize; x += g_ciBlockSize)
	{
		colorBuffer = (uint32_t*)mRT.back_buffer->pixels	+ frag->y * mRT.back_buffer->w + x;
		depthBuffer = (float*)mRT.z_buffer->pixels			+ frag->y * mRT.z_buffer->w + x;
		
		for (int y = frag->y; y < frag->y + g_ciTileSize; ++y)
		{
			CalcVaryings(f, x, y, W0, W1, WDY, V0, V1, VDY);
			SSE_ALIGN Arti3DPSParam ps_param;

			__m128	dbquad;
			__m128i dbmask, oquad, nquad;
			uint32_t	*colorTileLine = colorBuffer;
			float		*depthTileLine = depthBuffer;
			
			dbquad	= _mm_loadu_ps(depthTileLine);
			oquad	= _mm_loadu_si128((__m128i*)colorTileLine);

			// Depth test
			dbmask = *(__m128i*)&_mm_cmpge_ps(W0, dbquad);

			// If At Least 1 Pixels Passed Test.
			if (_mm_movemask_ps(*(__m128*)&dbmask))
			{
				PreInterpolateVaryings(W0, V0, ps_param.Varyings);
				mRC.pfnPS(&ps_param);

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

				mRC.pfnPS(&ps_param);

				nquad = ConvertColorFormat(ps_param.Output);

				// Update Depth Buffer!
				dbquad = _mm_or_ps(_mm_and_ps(W1, *(__m128*)&dbmask), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
				_mm_storeu_ps(depthTileLine, dbquad);

				nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
				_mm_storeu_si128((__m128i*)colorTileLine, nquad);
			}

			colorBuffer += mRT.back_buffer->w;
			depthBuffer += mRT.z_buffer->w;

			IncVaryingsAlongY(W0, W1, WDY, V0, V1, VDY);
		}
	}
}

void Arti3DDevice::RenderBlockFragments(Arti3DFragment *frag)
{
	__m128 W0, W1, WDY;
	__m128 V0[g_ciMaxVaryingNum], V1[g_ciMaxVaryingNum], VDY[g_ciMaxVaryingNum];
	__m128 C0 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
	__m128 C1 = _mm_set_ps1(4.0f);

	Arti3DTransformedFace *f = &faceBuffer[frag->faceID];

	float *depthBuffer = (float*)mRT.z_buffer->pixels + mRT.z_buffer->w * frag->y + frag->x;
	uint32_t *colorBuffer = (uint32_t *)mRT.back_buffer->pixels + mRT.back_buffer->w * frag->y + frag->x;

	for (int y = frag->y; y < frag->y + g_ciBlockSize; ++y)
	{
		CalcVaryings(f, frag->x, y, W0, W1, WDY, V0, V1, VDY);
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
			mRC.pfnPS(&ps_param);
			
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

			mRC.pfnPS(&ps_param);

			nquad = ConvertColorFormat(ps_param.Output);

			// Update Depth Buffer!
			dbquad = _mm_or_ps(_mm_and_ps(W1, *(__m128*)&dbmask), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
			_mm_storeu_ps(depthTileLine, dbquad);

			nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
			_mm_storeu_si128((__m128i*)colorTileLine, nquad);
		}

		colorBuffer += mRT.back_buffer->w;
		depthBuffer += mRT.z_buffer->w;

		IncVaryingsAlongY(W0, W1, WDY, V0, V1, VDY);
	}
}

void Arti3DDevice::PreInterpolateVaryings(__m128 &W, __m128 *iV, SSE_Float *oV)
{
	__m128 w = _mm_rcp_ps(W);
	for (int i = 0; i < g_ciMaxVaryingNum; ++i)
		oV[i] = _mm_mul_ps(w, iV[i]);
}

void Arti3DDevice::DrawTileGrid()
{
	int iWidth = mRT.back_buffer->w;
	int iHeight = mRT.back_buffer->h;

	uint32_t gridColor = ToyColor(1.0f, 1.0f, 1.0f).ToUInt32();

	//Draw Vertical Lines
	for (int x = 0; x < iWidth; x += g_ciTileSize)
		Draw2DLines(x, 0, x, iHeight - 1, gridColor);
	//Draw Horizontal Lines
	for (int y = 0; y < iHeight; y += g_ciTileSize)
		Draw2DLines(0, y, iWidth - 1, y, gridColor);
}

void Arti3DDevice::RenderMaskedFragments(Arti3DFragment *frag)
{
	__m128 W0, W1, WDY;
	__m128 V0[g_ciMaxVaryingNum], V1[g_ciMaxVaryingNum], VDY[g_ciMaxVaryingNum];
	__m128 C0 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
	__m128 C1 = _mm_set_ps1(4.0f);
	__m128i iMask = _mm_set_epi32(8, 4, 2, 1);

	Arti3DTransformedFace *f = &faceBuffer[frag->faceID];

	float *depthBuffer = (float*)mRT.z_buffer->pixels + mRT.z_buffer->w * frag->y + frag->x;
	uint32_t *colorBuffer = (uint32_t *)mRT.back_buffer->pixels + mRT.back_buffer->w * frag->y + frag->x;

	CalcVaryings(f, frag->x, frag->y, W0, W1, WDY, V0, V1, VDY);

	SSE_ALIGN Arti3DPSParam ps_param;
	
	__m128	dbquad;
	__m128i cbmask,dbmask, oquad, nquad;
	uint32_t	*colorTileLine = colorBuffer;
	float		*depthTileLine = depthBuffer;

	dbquad = _mm_loadu_ps(depthTileLine);
	oquad = _mm_loadu_si128((__m128i*)colorTileLine);

	cbmask = _mm_cmpgt_epi32(_mm_and_si128(_mm_set1_epi32(frag->mask), iMask),_mm_setzero_si128());

	// Depth test
	dbmask = *(__m128i*)&_mm_cmpge_ps(W0, dbquad);

	dbmask = _mm_and_si128(dbmask, cbmask);
	// If At Least 1 Pixels Passed Test.
	if (_mm_movemask_ps(*(__m128*)&dbmask))
	{
		PreInterpolateVaryings(W0, V0, ps_param.Varyings);
		mRC.pfnPS(&ps_param);

		nquad = ConvertColorFormat(ps_param.Output);

		// Update Depth Buffer!
		dbquad = _mm_or_ps(_mm_and_ps(W0, *(__m128*)&dbmask), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
		_mm_storeu_ps(depthTileLine, dbquad);

		nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
		_mm_storeu_si128((__m128i*)colorTileLine, nquad);
	}
}

Arti3DResult Arti3DDevice::InitializeDevice(Arti3DDeviceParameter deviceParam)
{
	m_iWidth = deviceParam.iWidth;
	m_iHeight = deviceParam.iHeight;

	if (deviceParam.bMultiThread)
		CreateWorkerThreads();

	InitTile();
	InitTileMT();
	return ARTI3D_OK;
}

void Arti3DDevice::ReleaseResource()
{
}

Arti3DResult Arti3DDevice::InitTileMT()
{

	m_iTileX = (m_iWidth + g_ciTileSize - 1) >> g_ciTileSizeShift;
	m_iTileY = (m_iHeight + g_ciTileSize - 1) >> g_ciTileSizeShift;

	uint32_t iTotal = m_iTileX * m_iTileY;

	m_pTiles = new Arti3DTile[m_iTileY * m_iTileX];

	if (!m_pTiles)
		return ARTI3D_OUT_OF_MEMORY;

	// Initialize Tile Data
	for (int y = 0; y < m_iTileY; ++y)
	{
		for (int x = 0; x < m_iTileX; ++x)
		{
			Arti3DTile &tile = m_pTiles[y * m_iTileX + x];
			tile.Create();
			tile.m_iX = x * g_ciTileSize;
			tile.m_iY = y * g_ciTileSize;
			if (x == m_iTileX - 1)
			{
				int iExtraX = m_iWidth & (g_ciTileSize - 1);
				tile.m_iWidth = iExtraX ? iExtraX : g_ciTileSize;
			}
			else
				tile.m_iWidth = g_ciTileSize;

			if (y == m_iTileY - 1)
			{
				int iExtraY = m_iHeight & (g_ciTileSize - 1);
				tile.m_iHeight = iExtraY ? iExtraY : g_ciTileSize;
			}
			else
				tile.m_iHeight = g_ciTileSize;
		}
	}

	// Allocate Space For Job Queue
	m_pJobQueue = new uint32_t[m_iTileX * m_iTileY];
	if (!m_pJobQueue)
		return ARTI3D_OUT_OF_MEMORY;
	m_iJobStart = 0;
	m_iJobEnd = 0;

	return ARTI3D_OK;
}

void Arti3DDevice::ClearTileMT()
{

}

Arti3DResult Arti3DDevice::SetVertexLayout(Arti3DVertexLayout *pLayout)
{
	if (!pLayout)
		return ARTI3D_INVALID_PARAMETER;
	
	m_pVertexLayout = pLayout;
	
	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::CreateWorkerThreads()
{
	m_pThreads = new Arti3DThread[g_ciMaxThreadNum];
	if (!m_pThreads)
		return ARTI3D_OUT_OF_MEMORY;
	for (int i = 0; i < g_ciMaxThreadNum; ++i)
		m_pThreads[i].Create(this,i); 

	m_iStage = 1;

	for (int i = 0; i < g_ciMaxThreadNum; ++i)
		m_vThread.push_back(std::thread(Arti3DThread::WorkFunc, &m_pThreads[i]));

	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::CreateVertexLayout(Arti3DVertexLayout **o_pVertexLayout, uint32_t iAttribute, Arti3DVertexAttributeFormat *i_pVAFormat)
{
	if (!o_pVertexLayout || iAttribute == 0 || !i_pVAFormat)
		return ARTI3D_INVALID_PARAMETER;

	*o_pVertexLayout = new Arti3DVertexLayout();

	if (!*o_pVertexLayout)
		return ARTI3D_OUT_OF_MEMORY;

	(*o_pVertexLayout)->Create(iAttribute, i_pVAFormat);

	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::CreateVertexBuffer(Arti3DVertexBuffer **o_pVertexBuffer, uint32_t iLength)
{
	if (!o_pVertexBuffer || iLength == 0)
		return ARTI3D_INVALID_PARAMETER;

	*o_pVertexBuffer = new Arti3DVertexBuffer();

	if (!*o_pVertexBuffer)
		return ARTI3D_OUT_OF_MEMORY;

	(*o_pVertexBuffer)->Create(iLength);

	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::SetVertexBuffer(Arti3DVertexBuffer *pVertexBuffer)
{
	if (!pVertexBuffer)
		return ARTI3D_NULL_PARAMETER;

	m_pVertexBuffer = pVertexBuffer;

	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::CreateIndexBuffer(Arti3DIndexBuffer **o_pIndexBuffer, uint32_t iLength, Arti3DFormat format)
{
	if (!o_pIndexBuffer)
		return ARTI3D_NULL_PARAMETER;
	if (!iLength)
		return ARTI3D_INVALID_PARAMETER;

	*o_pIndexBuffer = new Arti3DIndexBuffer();
	
	if (!*o_pIndexBuffer)
		return ARTI3D_OUT_OF_MEMORY;

	(*o_pIndexBuffer)->Create(iLength, format);
	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::SetIndexBuffer(Arti3DIndexBuffer *pIndexBuffer)
{
	if (!pIndexBuffer)
		return ARTI3D_NULL_PARAMETER;
	m_pIndexBuffer = pIndexBuffer;
	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::PreRender()
{
	return ARTI3D_OK;
}

void Arti3DDevice::StopAllThreads()
{
	m_bThreadStop = true;
	for (auto& x : m_vThread)
		x.join();
	printf("All Thread Exited.\n");
}

void Arti3DDevice::DrawMesh_MT()
{
	// Clear Buffers, Local Or Global.


	// Activate all threads
	// !!!!! Note !!!!!
	// It's Important That The Following Line Comes First! 
	m_iWorkingThread = g_ciMaxThreadNum;
	
	// It's Important That The Following Line Comes The Last.
	m_iStage = 0;
	
	
	// Wait For This Round To End.
	while (!(m_iStage == 1 && m_iWorkingThread == 0))
		std::this_thread::yield();

	ClearJobQueue();
	
	uint32_t iTileNum = m_iTileX * m_iTileY;

	for (uint32_t i = 0; i < iTileNum; ++i)
	{
		m_pTiles[i].Clear();
	}
#ifdef LogoutInfo
	printf("All Threads Finishes This Round!\n");
#endif
}

Arti3DResult Arti3DDevice::InitializeWorkThreads()
{
	// Allocate Work Load For Every Threads.
	uint32_t iNumTriangle = m_pIndexBuffer->iGetIndexNum() / 3;
	if (m_pIndexBuffer->iGetIndexNum() % 3 != 0)
		return ARTI3D_INVALID_BUFFER_SIZE;

	uint32_t iFacePerThread = iNumTriangle / g_ciMaxThreadNum;
	uint32_t iExtraFace = iNumTriangle % g_ciMaxThreadNum;

	uint32_t workLoad[g_ciMaxThreadNum];
	
	for (auto& x : workLoad)
		x = iFacePerThread;

	if (iExtraFace)
	{
		for (uint32_t i = 0; i < iExtraFace; ++i)
			++workLoad[i];
	}

	uint32_t iStart = 0, iEnd = 0;

	for (uint32_t i = 0; i < g_ciMaxThreadNum; ++i)
	{
		iEnd = iStart + workLoad[i] * 3 - 1;
		m_pThreads[i].DistributeWorkLoad(i, iStart, iEnd);
		iStart = iEnd + 1;
	}

	return ARTI3D_OK;
}

void Arti3DDevice::ClearJobQueue()
{
	m_iJobStart = m_iJobStart2 = m_iJobEnd = 0;
}

