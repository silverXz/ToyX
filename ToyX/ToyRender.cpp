#include "stdafx.h"
#include "ToyRender.h"
#include <algorithm>
#include <cstdlib>
#include <assert.h>

using namespace toy;

ToyRender::ToyRender()
{
}

ToyRender::~ToyRender()
{
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
void ToyRender::SetPixelColor(int x, int y, const ToyColor& color)
{
	assert(x >= 0 && y >= 0);
	if (x >= mRT.back_buffer->w || y >= mRT.back_buffer->h)
		return;
	((Uint32*)mRT.back_buffer->pixels)[y * mRT.back_buffer->w + x] = color.ToUInt32();
}

void ToyRender::DrawHorizontal2DLine(int x1, int x2, int y, const ToyColor& color)
{
	for (int i = x1; i <= x2; ++i)
		SetPixelColor(i, y, color);
}

void ToyRender::DrawVertical2DLine(int y1, int y2, int x, const ToyColor& color)
{
	for (int i = y1; i <= y2; ++i)
		SetPixelColor(x, i, color);
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

bool ToyRender::EdgeFunc(int a, int b, int c, int x, int y)
{
	if (a * x + b * y + c >= 0)
		return true;
	else
		return false;
}
void ToyRender::LoadCube()
{
	Toy_Vertex v[6];
	v[0].p = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	v[0].c = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	v[1].p = vec4(-1.0f, 0.0f, 0.0f, 1.0f);
	v[1].c = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	v[2].p = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	v[2].c = vec4(0.0f, 0.0f, 1.0f, 1.0f);

	v[3].p = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	v[3].c = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	v[4].p = vec4(-1.0f, 0.0f, -1.0f, 1.0f);
	v[4].c = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	v[5].p = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	v[5].c = vec4(1.0f, 1.0f, 1.0f, 1.0f);


	UploadData(GEOMETRY_VERTEX, v, sizeof(v));

	uint32_t indices[6] = { 0, 1, 2,3,4,5 };
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


void ToyRender::ProcessV()
{
	for (int i = 0; i < iBuffer.size / 3; ++i)
	{
		Toy_TransformedFace face;
		for (int j = 0; j < 3 ; ++j)
		{
			int k = 3 * i + j;
			int vid = iBuffer.iBuffer[k];
			// Found Vertex In Cache!
			if (vCache[vid &(CACHE_SIZE - 1)].tag == vid)
			{
				face.v[j] = &tvBuffer[vid];
			}
			else
			{
				VS_PARAM vs_param;
				vs_param.v_in = &vBuffer.vBuffer[vid];
				vs_param.v_out = &tvBuffer[vid];
				vs_param.uniforms = &mRC.globals;
				mRC.vs(&vs_param);
				PostProcessV(vs_param.v_out);
				vCache[vid&(CACHE_SIZE - 1)].v = vs_param.v_out;
				vCache[vid&(CACHE_SIZE - 1)].tag = vid;
				face.v[j] = vs_param.v_out;
			}
		}
		faceBuffer.push_back(face);
	}
}

void ToyRender::ProcessR()
{
	for (int i = 0; i < faceBuffer.size(); ++i)
	{
		RasterizeTriangle(&faceBuffer[i]);
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

void ToyRender::RasterizeTriangle(Toy_TransformedFace* f)
{
	Toy_TransformedVertex *v0 = f->v[0];
	Toy_TransformedVertex *v1 = f->v[1];
	Toy_TransformedVertex *v2 = f->v[2];

	// Fixed Point Coordinates.
	f->fp1[0] = iRound(v0->p.x * 16.0f);
	f->fp1[1] = iRound(v0->p.y * 16.0f);
	f->fp2[0] = iRound(v1->p.x * 16.0f);
	f->fp2[1] = iRound(v1->p.y * 16.0f);
	f->fp3[0] = iRound(v2->p.x * 16.0f);
	f->fp3[1] = iRound(v2->p.y * 16.0f);

	int DX21 = f->fp2[0] - f->fp1[0];
	int DY21 = f->fp2[1] - f->fp1[1];
	int DX32 = f->fp3[0] - f->fp2[0];
	int DY32 = f->fp3[1] - f->fp2[1];
	int DX13 = f->fp1[0] - f->fp3[0];
	int DY13 = f->fp1[1] - f->fp3[1];

	int faceOrient = DX13*DY21-DX21*DY13;
	
	if (faceOrient > 0)
		return;

	// compute gradient
	f->v0x = v0->p.x;
	f->v0y = v0->p.y;
	f->v0w = v0->p.w;

	float fdx21 = v1->p.x - v0->p.x;
	float fdx31 = v2->p.x - v0->p.x;
	float fdy21 = v1->p.y - v0->p.y;
	float fdy31 = v2->p.y - v0->p.y;

	float fdw21 = v1->p.w - v0->p.w;
	float fdw31 = v2->p.w - v0->p.w;

	float CS = fdx21 * fdy31 - fdx31 * fdy21;
	ComputeGradient(CS, v1->p.w - v0->p.w, v2->p.w - v0->p.w, fdx21, fdy21, fdx31, fdy31, &f->dw);

	for (int i = 0; i < VARYINGS_NUM; ++i)
	{
		ComputeGradient(CS, v1->varyings[i] - v0->varyings[i], v2->varyings[i] - v0->varyings[i], fdx21, fdy21, fdx31, fdy31, &f->dv[i]);
	}


	int C1 = -DY21 * f->fp1[0] + DX21 * f->fp1[1];
	int C2 = -DY32 * f->fp2[0] + DX32 * f->fp2[1];
	int C3 = -DY13 * f->fp3[0] + DX13 * f->fp3[1];

	if (DY21 > 0 || (DY21 == 0 && DX21 > 0))
		++C1;
	if (DY32 > 0 || (DY32 == 0 && DX32 > 0))
		++C2;
	if (DY13 > 0 || (DY13 == 0 && DX13 > 0))
		++C3;

	int DDX21 = DX21 << 4;
	int DDX32 = DX32 << 4;
	int DDX13 = DX13 << 4;
	int DDY21 = DY21 << 4;
	int DDY32 = DY32 << 4;
	int DDY13 = DY13 << 4;

	int xmin = std::min(std::min(f->fp1[0], f->fp2[0]), f->fp3[0]);
	int xmax = std::max(std::max(f->fp1[0], f->fp2[0]), f->fp3[0]);
	int ymin = std::min(std::min(f->fp1[1], f->fp2[1]), f->fp3[1]);
	int ymax = std::max(std::max(f->fp1[2], f->fp2[1]), f->fp3[1]);

 	int E1 = DY21 * xmin - DX21 * ymin + C1;
	int E2 = DY32 * xmin - DX32 * ymin + C2;
	int E3 = DY13 * xmin - DX13 * ymin + C3;

	int cxMin = xmin >> 4;
	int cxMax = xmax >> 4;
	int cyMin = ymin >> 4;
	int cyMax = ymax >> 4;

	for (int y = cyMin; y <= cyMax; ++y)
	{
		int e1 = E1, e2 = E2, e3 = E3;
		for (int x = cxMin; x <= cxMax; ++x)
		{
			if (e1 > 0 && e2 > 0 && e3 > 0)
			{
				float xStep = x - f->v0x;
				float yStep = y - f->v0y;
				float cw = f->v0w + xStep * f->dw.x + yStep * f->dw.y;	

				if (cw >= ((float*)mRT.z_buffer->pixels)[y * mRT.z_buffer->w + x])
				{
					float rcw = 1.0f / cw;
					float cx = f->v[0]->varyings[0] + xStep * f->dv[0].x + yStep * f->dv[0].y;
					cx *= rcw;
					float cy = f->v[0]->varyings[1] + xStep * f->dv[1].x + yStep * f->dv[1].y;
					cy *= rcw;
					float cz = f->v[0]->varyings[2] + xStep * f->dv[2].x + yStep * f->dv[2].y;
					cz *= rcw;
					SetPixelColor(x, y, ToyColor(cx, cy, cz));
					((float*)mRT.z_buffer->pixels)[y * mRT.z_buffer->w + x] = cw;
				}
			}
			e1 += DDY21;
			e2 += DDY32;
			e3 += DDY13;
		}
		E1 -= DDX21;
		E2 -= DDX32;
		E3 -= DDX13;
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

void ToyRender::ComputeGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, toy::vec2 *g)
{
	float A = di21 * dy31 - di31 * dy21;
	float B = di21 * dx31 - di31 * dx21;
	g->x = A / C;
	g->y = -B / C;
}
