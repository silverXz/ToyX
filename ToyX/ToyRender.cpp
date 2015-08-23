#include "stdafx.h"
#include "ToyRender.h"
#include <algorithm>
#include <cstdlib>
#include <assert.h>
#include <xmmintrin.h>
#include <smmintrin.h>

using namespace toy;

ToyRender::ToyRender()
{
	dFile.open("Debug.txt", std::ios::ate);
}

ToyRender::~ToyRender()
{
	dFile.close();
}



void ToyRender::ClearColorBuffer(const ToyColor& color)
{
	int hr = SDL_FillRect(mRT.back_buffer, nullptr, color.ToUInt32());
	if (hr)
		std::cerr << "Failed to clear color buffer!\n";
}

void ToyRender::ClearDepthBuffer(float cDepth)
{
	SDL_FillRect(mRT.z_buffer, nullptr, 0);
}

void ToyRender::SetMatrix(MatrixType matrixType, const toy::mat4& m)
{
	GlobalUniforms &rgu = mRC.globals;
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

void ToyRender::SetViewport(int x, int y, int width, int height)
{
	mRC.globals.viewport = vec4(FLOAT_CAST(x),FLOAT_CAST(y), FLOAT_CAST(width), FLOAT_CAST(height));
}

void ToyRender::Draw2DLines(int x1, int y1, int x2, int y2, const ToyColor &color)
{
	int dy = abs(y2 - y1);
	int dx = abs(x2 - x1);
	if (dx == 0 && dy == 0)
		SetPixelColor(x1, y1, color.ToUInt32());
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
		SetPixelColor(curx, cury, color.ToUInt32());
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

void ToyRender::Draw3DLines(const toy::vec4& p1, const toy::vec4 p2, const ToyColor &color)
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
	Draw2DLines(clip1.x, clip1.y, clip2.x, clip2.y, color);
}

void ToyRender::Draw3DSolidTriangle(const toy::vec4& p1, const toy::vec4& p2, const toy::vec4& p3, const ToyColor& c)
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

void ToyRender::LoadCube()
{
	const float len = 2.0f;
	Toy_Vertex v[8];
	
	v[0].p = vec4(-len, len, len, 1.0f);
	v[0].c = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	v[1].p = vec4(len, len, len, 1.0f);
	v[1].c = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	
	v[2].p = vec4(len, len, -len, 1.0f);
	v[2].c = vec4(0.0f, 0.0f, 1.0f, 1.0f);

	v[3].p = vec4(-len, len, -len, 1.0f);
	v[3].c = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	
	v[4].p = vec4(-len, -len, len, 1.0f);
	v[4].c = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	
	v[5].p = vec4(len, -len, len, 1.0f);
	v[5].c = vec4(1.0f, 1.0f, 0.0f, 1.0f);

	v[6].p = vec4(len, -len, -len, 1.0f);
	v[6].c = vec4(1.0f,1.0f,1.0f,0.0f);

	v[7].p = vec4(-len, -len, -len, 1.0f);
	v[7].c = vec4(0.0f, 0.0f, 0.0f, 1.0f);


	UploadData(GEOMETRY_VERTEX, v, sizeof(v));

	uint32_t indices[] = { 0,1,2,0,2,3,0,4,5,0,5,1,1,5,6,1,6,2,4,7,6,4,6,5,0,3,7,0,7,4,3,2,6,3,6,7};
	//uint32_t indices[] = { 1, 5, 6, 1, 6, 2 };
	UploadData(GEOMETRY_INDICE, indices, sizeof(indices));
}

void ToyRender::Begin()
{
	if (!mRT.back_buffer)
	{
		std::cerr << "ToyRender::Begin(): No Backbuffer!\n";
		return;
	}
	SDL_LockSurface(mRT.back_buffer);
}

void ToyRender::End()
{
	SDL_UnlockSurface(mRT.back_buffer);
}

void ToyRender::SetRenderTarget(SDL_Surface *cb, SDL_Surface *zb)
{
	mRT.back_buffer = cb;
	mRT.z_buffer = zb;
}


void ToyRender::UploadData(GeometryDataType gdt, void *ptr, uint32_t size)
{
	if (!ptr)
	{
		std::cerr << "UploadData : nullptr!\n";
		return;
	}

	switch (gdt)
	{
	case GEOMETRY_VERTEX:
		memcpy(vBuffer.vBuffer, ptr, size);
		vBuffer.size = size / sizeof(Toy_Vertex);
		break;
	case GEOMETRY_INDICE:
		memcpy(iBuffer.iBuffer, ptr, size);
		iBuffer.size = size / sizeof(uint32_t);
		break;
	default:
		break;
	}
}

void ToyRender::TransformVertex(uint32_t in, Toy_TransformedVertex *out)
{
	uint32_t cacheID = in&(CACHE_SIZE - 1);
	if (vCache[cacheID].tag == in)
	{
		*out = *(vCache[cacheID].v);
	}
	else
	{
		VS_PARAM vs_param;
		vs_param.v_in = &vBuffer.vBuffer[in];
		vs_param.v_out = &tvBuffer[in];
		vs_param.uniforms = &mRC.globals;
		mRC.vs(&vs_param);
		vCache[cacheID].tag = in;
		vCache[cacheID].v = vs_param.v_out;
		*out = *(vCache[cacheID].v);
	}
}

void ToyRender::ProcessV()
{
	faceBuffer.clear();
	ClearCache();

	for (int i = 0; i < iBuffer.size; i += 3)
	{
		// For every face, get transformed vertex. 
		// Clipping may happen that new vertex is introduced.
		// The maximum vertex number clipping may produces is 
		Toy_TransformedVertex v[CLIP_VERTEX_MAX];
		for (int j = 0; j < 3; ++j)
		{
			TransformVertex(iBuffer.iBuffer[i + j], &v[j]);
			PostProcessV(&v[j]);
		}
		InsertTransformedFace(&v[0], &v[1], &v[2]);
	}
}

void ToyRender::ProcessV_WithClip()
{
	faceBuffer.clear();
	ClearCache();

	for (int i = 0; i < iBuffer.size; i += 3)
	{
		// For every face, get transformed vertex. 
		// Clipping may happen that new vertex is introduced.
		// The maximum vertex number clipping may produces is 
		Toy_TransformedVertex v[CLIP_VERTEX_MAX];
		for (int j = 0; j < 3; ++j)
			TransformVertex(iBuffer.iBuffer[i + j], &v[j]);
		ClipTriangle(&v[0], &v[1], &v[2]);
	}
}

void ToyRender::ClipTriangle(Toy_TransformedVertex *v1, Toy_TransformedVertex *v2, Toy_TransformedVertex *v3)
{
	int mask = 0; 
	mask |= CalcClipMask(v1);
	mask |= CalcClipMask(v2);
	mask |= CalcClipMask(v3);

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

	ClipMask mk[6] = {
		CLIP_POS_X,
		CLIP_NEG_X,
		CLIP_POS_Y,
		CLIP_NEG_Y,
		CLIP_POS_Z,
		CLIP_NEG_Z
	};

	// We need 2 index array ( with size CLIP_VERTEX_MAX ) to do ping pong buffering.
	Toy_TransformedVertex *v = v1;
	uint32_t inout[2][CLIP_VERTEX_MAX];
	uint32_t *in = inout[0], *out = inout[1];
	in[0] = 0;	in[1] = 1; in[2] = 2;

	int vCnt = 3;
	int inCnt = 3;
	int outCnt = 0;

	// Several lambda expression to help!

	// Calculate the signed distance between a vertex and a plane.
	auto calcPointPlaneDistance = [](const Toy_Plane *p, const Toy_TransformedVertex *v) { 	return p->x * v->p.x + p->y * v->p.y + p->z * v->p.z + p->d;};

	// Determine whether two floats has different signs.
	auto hasDifferentSigns = [](float a, float b) { return (a >= 0.0f && b < 0.0f) || (a < 0.0f && b >= 0.0f);	};

	// Interpolate Vertex Attributes
	auto interpolateV = [](const Toy_TransformedVertex *v1, const Toy_TransformedVertex *v2, float t, Toy_TransformedVertex *out) {
		out->p = v1->p + (v2->p - v1->p) * t;
		for (int i = 0; i < VARYINGS_NUM; ++i)
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
					out[outCnt++] = d1;
				
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
		}
		
		std::swap(in, out);

		inCnt = outCnt;
		outCnt = 0;
	}

	for (int i = 0; i < inCnt; ++i)
		PostProcessV(&v[in[i]]);

	for (int i = 1; i < inCnt - 1; ++i)
		InsertTransformedFace(&v[in[0]], &v[in[i]], &v[in[i + 1]]);
}

void ToyRender::InsertTransformedFace(Toy_TransformedVertex *v1, Toy_TransformedVertex *v2, Toy_TransformedVertex *v3)
{
	Toy_TransformedFace f;
	
	f.v0x = v1->p.x;
	f.v0y = v1->p.y;
	f.v0w = v1->p.w;

	for (int i = 0; i < VARYINGS_NUM; ++i)
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

	for (int i = 0; i < VARYINGS_NUM; ++i)
	{
		ComputeGradient(CS, v2->varyings[i] - v1->varyings[i], v3->varyings[i] - v1->varyings[i], fdx21, fdy21, fdx31, fdy31, &f.dv[i]);
	}

	faceBuffer.push_back(f);
}

int ToyRender::CalcClipMask(Toy_TransformedVertex *v)
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

void ToyRender::ProcessR()
{
	for (int i = 0; i < faceBuffer.size(); ++i)
	{
		RasterizeTriangle_SIMD(&faceBuffer[i]);
	}
}

void ToyRender::PostProcessV(Toy_TransformedVertex *v)
{
	float invW = 1.0f / v->p.w;
	v->p.x *= invW;
	v->p.y *= invW;
	v->p.w = invW;

	// Transform to screen space.
	v->p.x = (v->p.x + 1.0f) * 0.5f * mRT.back_buffer->w;
	v->p.y = mRT.back_buffer->h - (v->p.y + 1.0f)*0.5f *mRT.back_buffer->h;

	for (int j = 0; j < VARYINGS_NUM; ++j)
	{
		v->varyings[j] *= invW;
	}
}

void ToyRender::DrawMesh()
{
	ProcessV();
	
	ProcessR();
}

void ToyRender::SetVertexShader(VertexShader vs)
{
	mRC.vs = vs;
}


void ToyRender::SetFragmentShader(FragmentShader fs)
{
	mRC.fs = fs;
}


void ToyRender::ComputeGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, toy::vec2 *g)
{
	float A = di21 * dy31 - di31 * dy21;
	float B = di21 * dx31 - di31 * dx21;
	g->x = A / C;
	g->y = -B / C;
}

void ToyRender::ClearCache()
{
	for (auto i = 0; i < CACHE_SIZE; ++i)
	{
		vCache[i].Clear();
	}
}


void ToyRender::RasterizeTriangle_SIMD(Toy_TransformedFace *f)
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

	int cxMin = (xmin + 0xF) >> 4;
	cxMin &= ~(blockSize - 1);
	int cxMax = (xmax + 0xF) >> 4;
	cxMax = (cxMax + blockSize) & (~(blockSize - 1));
	int cyMin = (ymin + 0xF) >> 4;
	cyMin &= ~(blockSize - 1);
	int cyMax = (ymax + 0xF) >> 4;
	cyMax = (cyMax + blockSize) & (~(blockSize - 1));

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

			// Test Against Egde 1:
			bool e00 = (C1 + DY21 * x0 - DX21 * y0) > 0;
			bool e01 = (C1 + DY21 * x0 - DX21 * y1) > 0;
			bool e02 = (C1 + DY21 * x1 - DX21 * y0) > 0;
			bool e03 = (C1 + DY21 * x1 - DX21 * y1) > 0;

			int a1 = (e00 << 0) | (e01 << 1) | (e02 << 2) | (e03 << 3);

			// Test Against Edge 2:
			bool e10 = (C2 + DY32 * x0 - DX32 * y0) > 0;
			bool e11 = (C2 + DY32 * x0 - DX32 * y1) > 0;
			bool e12 = (C2 + DY32 * x1 - DX32 * y0) > 0;
			bool e13 = (C2 + DY32 * x1 - DX32 * y1) > 0;

			int a2 = (e10 << 0) | (e11 << 1) | (e12 << 2) | (e13 << 3);

			// Test Against Edge 3:
			bool e20 = (C3 + DY13 * x0 - DX13 * y0) > 0;
			bool e21 = (C3 + DY13 * x0 - DX13 * y1) > 0;
			bool e22 = (C3 + DY13 * x1 - DX13 * y0) > 0;
			bool e23 = (C3 + DY13 * x1 - DX13 * y1) > 0;

			int a3 = (e20 << 0) | (e21 << 1) | (e22 << 2) | (e23 << 3);

			// Complete Outside Triangle!
			if (a1 == 0 || a2 == 0 || a3 == 0)
				continue;

			// Totally Inside, Handle blockSize * blockSize PIxels
			if (a1 == 0xF && a2 == 0xF && a3 == 0xF)
			{
				__m128 W0, W1, WDY;
				__m128 V0[VARYINGS_NUM], V1[VARYINGS_NUM], VDY[VARYINGS_NUM];
				__m128 C0 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
				__m128 C1 = _mm_set_ps1(4.0f);
				__m128 fMax = _mm_set_ps1(255.0f);
				//__m128i mask_mask = _mm_set_epi32(8, 4, 2, 1);

				float *depthBuffer = mRT.z_buffer ? (float*)mRT.z_buffer->pixels + y * mRT.z_buffer->w + x : nullptr;
				uint32_t *colorBuffer = (uint32_t*)mRT.back_buffer->pixels + y * mRT.back_buffer->w + x;

				float dxStart = x - f->v0x;
				float dyStart = y - f->v0y;

				__m128 dx = _mm_set1_ps(f->dw.x);
				__m128 base = _mm_set1_ps(f->v0w + f->dw.x * dxStart + f->dw.y * dyStart);

				WDY = _mm_set1_ps(f->dw.y);
				W0 = _mm_add_ps(base, _mm_mul_ps(dx, C0));
				W1 = _mm_add_ps(W0, _mm_mul_ps(dx, C1));

				/*Compute Varyings*/
				for (int i = 0; i < VARYINGS_NUM; ++i)
				{
					base = _mm_set1_ps(f->v0v[i] + dxStart * f->dv[i].x + dyStart * f->dv[i].y);
					dx = _mm_set1_ps(f->dv[i].x);
					VDY[i] = _mm_set1_ps(f->dv[i].y);
					V0[i] = _mm_add_ps(base, _mm_mul_ps(dx, C0));
					V1[i] = _mm_add_ps(V0[i], _mm_mul_ps(dx, C1));
				}

				for (int iy = 0; iy < blockSize; iy++)
				{
					SSE_ALIGN PS_PARAM parm;
					parm.uniforms = &mRC.globals;
					__m128 dbquad;
					__m128i oquad, nquad, dbmask;
					uint32_t *cbTileLine;
					float *dbTileLine = nullptr;

					dbTileLine = depthBuffer;
					dbquad = _mm_loadu_ps(dbTileLine);

					cbTileLine = colorBuffer;

					oquad = _mm_loadu_si128((__m128i*)cbTileLine);

					dbmask = *(__m128i*)&_mm_cmpge_ps(W0, dbquad);

					// Not Zero : means at least 1 pixel passed the depth test!
					if (_mm_movemask_ps(*(__m128*)&dbmask))
					{
						__m128 w = _mm_rcp_ps(W0);
						for (int f = 0; f < VARYINGS_NUM; f += 2)
						{
							parm.Varyings[f + 0] = _mm_mul_ps(w, V0[f + 0]);
							parm.Varyings[f + 1] = _mm_mul_ps(w, V0[f + 1]);
						}
						mRC.fs(&parm);

						SSE_Vec3 &out = parm.Output;

						out.r = _mm_min_ps(_mm_mul_ps(out.r.f, fMax), fMax);
						out.g = _mm_min_ps(_mm_mul_ps(out.g.f, fMax), fMax);
						out.b = _mm_min_ps(_mm_mul_ps(out.b.f, fMax), fMax);

						__m128i iR = _mm_cvtps_epi32(out.r.f);
						__m128i iG = _mm_cvtps_epi32(out.g.f);
						__m128i iB = _mm_cvtps_epi32(out.b.f);

						iR = _mm_slli_epi32(iR, 16);
						iG = _mm_slli_epi32(iG, 8);

						nquad = _mm_or_si128(_mm_or_si128(iR, iG), iB);

						// Store Results

						// Store Depth Info.
						dbquad = _mm_or_ps(_mm_and_ps(*(__m128*)&dbmask, W0), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
						_mm_storeu_si128((__m128i*)dbTileLine, *(__m128i*)&dbquad);

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
						__m128 w = _mm_rcp_ps(W1);
						for (int f = 0; f < VARYINGS_NUM; f += 2)
						{
							parm.Varyings[f + 0] = _mm_mul_ps(w, V1[f + 0]);
							parm.Varyings[f + 1] = _mm_mul_ps(w, V1[f + 1]);
						}
						mRC.fs(&parm);

						SSE_Vec3 &out = parm.Output;

						out.r = _mm_min_ps(_mm_mul_ps(out.r.f, fMax), fMax);
						out.g = _mm_min_ps(_mm_mul_ps(out.g.f, fMax), fMax);
						out.b = _mm_min_ps(_mm_mul_ps(out.b.f, fMax), fMax);

						__m128i iR = _mm_cvtps_epi32(out.r.f);
						__m128i iG = _mm_cvtps_epi32(out.g.f);
						__m128i iB = _mm_cvtps_epi32(out.b.f);

						iR = _mm_slli_epi32(iR, 16);
						iG = _mm_slli_epi32(iG, 8);

						nquad = _mm_or_si128(_mm_or_si128(iR, iG), iB);

						// Store Results

						// Store Depth Info.
						dbquad = _mm_or_ps(_mm_and_ps(*(__m128*)&dbmask, W0), _mm_andnot_ps(*(__m128*)&dbmask, dbquad));
						_mm_storeu_si128((__m128i*)dbTileLine, *(__m128i*)&dbquad);

						// Store Color Info.
						nquad = _mm_or_si128(_mm_and_si128(dbmask, nquad), _mm_andnot_si128(dbmask, oquad));
						_mm_storeu_si128((__m128i*)cbTileLine, nquad);
					}

					// Util now, this blockSize pixels has been dealt with. Get Ready for the next 8.
					W0 = _mm_add_ps(W0, WDY);
					W1 = _mm_add_ps(W1, WDY);

					for (int sb = 0; sb < VARYINGS_NUM; sb += 2)
					{
						V0[sb + 0] = _mm_add_ps(V0[sb + 0], VDY[sb + 0]);
						V0[sb + 1] = _mm_add_ps(V0[sb + 1], VDY[sb + 1]);
						V1[sb + 0] = _mm_add_ps(V1[sb + 0], VDY[sb + 0]);
						V1[sb + 1] = _mm_add_ps(V1[sb + 1], VDY[sb + 1]);
					}

					colorBuffer += mRT.back_buffer->w;
					depthBuffer += mRT.z_buffer->w;
				}
				continue;
			}

			// partially inside.
			int xe1 = DY21 * x0 - DX21 * y0 + C1;
			int xe2 = DY32 * x0 - DX32 * y0 + C2;
			int xe3 = DY13 * x0 - DX13 * y0 + C3;

			for (int i = y; i < y + blockSize; ++i)
			{
				int e1 = xe1, e2 = xe2, e3 = xe3;
				for (int j = x; j < x + blockSize; ++j)
				{
					if (e1 > 0 && e2 > 0 && e3 > 0)
					{
						float xStep = j - f->v0x;
						float yStep = i - f->v0y;
						float cw = f->v0w + xStep * f->dw.x + yStep * f->dw.y;

						if (cw >= ((float*)mRT.z_buffer->pixels)[i * mRT.z_buffer->w + j])
						{
							float rcw = 1.0f / cw;
							float cx = f->v0v[0] + xStep * f->dv[0].x + yStep * f->dv[0].y;
							cx *= rcw;
							float cy = f->v0v[1] + xStep * f->dv[1].x + yStep * f->dv[1].y;
							cy *= rcw;
							float cz = f->v0v[2] + xStep * f->dv[2].x + yStep * f->dv[2].y;
							cz *= rcw;
							ToyColor c(cx, cy, cz);
							//ToyColor c(1.0f, 1.0f, 1.0f);
							SetPixelColor(j, i, c.ToUInt32());
							((float*)mRT.z_buffer->pixels)[i * mRT.z_buffer->w + j] = cw;
						}
					}
					e1 += DDY21;
					e2 += DDY32;
					e3 += DDY13;
				}
				xe1 -= DDX21;
				xe2 -= DDX32;
				xe3 -= DDX13;
			}
		}
	}
}