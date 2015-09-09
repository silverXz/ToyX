#pragma once

#include <stdint.h>
#include <SDL/SDL.h>
#include <vector>
#include <fstream>
#include "ToyMath.h"
#include "Shader.h"
#include "Arti3D_Types.h"
#include "Arti3D_ForwardDecl.h"


class Arti3DDevice
{

public:



public:
	Arti3DDevice();
	~Arti3DDevice();


	void InitTile();

	void IntiTileMT();

	void SetRenderTarget(const RenderTarget& rRT);

	Arti3DResult InitializeDevice(Arti3DDeviceParameter deviceParam);


	void Begin();

	void End();

	// Draw Calls

	// Bresenham algorithm
	void Draw2DLines(int x1, int y1, int x2, int y2, uint32_t color);


	void Draw3DLines(const toy::vec4& p1, const toy::vec4 p2, uint32_t color);

	inline void SetPixelColor(int x, int y, uint32_t c)
	{
		assert(x >= 0 && y >= 0);
		if (x >= mRT.back_buffer->w || y >= mRT.back_buffer->h)
			return;
		((uint32_t*)mRT.back_buffer->pixels)[y * mRT.back_buffer->w + x] = c;
	}



	Arti3DResult InitializeThreads();

	Arti3DResult CreateVertexLayout(Arti3DVertexLayout **o_pVertexLayout, uint32_t iAttribute, Arti3DVertexAttributeFormat *i_pVAFormat);
	
	Arti3DResult SetVertexLayout(Arti3DVertexLayout *pLayout);

	Arti3DResult CreateVertexBuffer(Arti3DVertexBuffer **o_pVertexBuffer, uint32_t iLength);

	Arti3DResult SetVertexBuffer(Arti3DVertexBuffer *pVertexBuffer);

	Arti3DResult CreateIndexBuffer(Arti3DIndexBuffer **o_pIndexBuffer, uint32_t iLength,Arti3DFormat format);

	Arti3DResult SetIndexBuffer(Arti3DIndexBuffer *pIndexBuffer);

	void SetVertexShader(VertexShader vs);
	void SetFragmentShader(FragmentShader fs);

	// Render State Related
	void SetMatrix(Arti3DMatrixType matrixType, const toy::mat4& m);
	void SetViewport(int x, int y, int width, int height);

	void ClearColorBuffer(const ToyColor& color);
	void ClearDepthBuffer(float cDepth = 0.0f);

	void Draw3DSolidTriangle(const toy::vec4& p1,const toy::vec4& p2,const toy::vec4& p3,const ToyColor& c);

	void UploadData(GeometryDataType gdt, void *ptr, uint32_t size);

	void LoadCube();

	void DrawMesh();

	void DrawMesh_TileBase();

private:


	// For Debug Use.
	void DrawTileGrid();

	// Rasterize Triangle With SIMD Instructions	
	void RasterizeTriangle_SIMD(Arti3DTransformedFace *f);

	void Tilize(uint32_t faceid);

	void ProcessV_WithClip();
	
	// Process Vertex!
	void ProcessV();
	void GetTransformedVertex(uint32_t in, Arti3DTransformedVertex *out);
	void PostProcessV(Arti3DTransformedVertex *v);

	// Clipping Functions!
	inline int CalcClipMask(Arti3DTransformedVertex *v);
	void ClipTriangle(Arti3DTransformedVertex *v1, Arti3DTransformedVertex *v2, Arti3DTransformedVertex *v3);
	void InsertTransformedFace(Arti3DTransformedVertex *v1, Arti3DTransformedVertex *v2, Arti3DTransformedVertex *v3);

	// Process Rasterization!
	void ProcessR();

	void RasterizeTile();

	void RenderFragments();

	void RenderTileFragments(Arti3D_Fragment *frag);
	void RenderBlockFragments(Arti3D_Fragment *frag);
	void RenderMaskedFragments(Arti3D_Fragment *frag);

	inline void CalcVaryings(Arti3DTransformedFace* f,int x,int y,__m128 &W0,__m128 &W1,__m128 &WDY,__m128 *V0,__m128 *V1,__m128 *VDY);
	inline void PreInterpolateVaryings(__m128 &W, __m128 *iV, SSE_Float *oV);
	inline void IncVaryingsAlongY(__m128 &W0, __m128 &W1, __m128 WDY, __m128 *V0, __m128 *V1, __m128 *VDY);
	inline __m128i ConvertColorFormat(SSE_Color3 &src);
	inline void UpdateRenderTarget();

	// Process Pixels!
	// !Not implemented yet!
	void ProcessP();

	
	
	// Helper Functions
	inline int iRound(float f)
	{
		int retval;
		__asm {
			fld f
			fistp retval
		}
		return retval;
		//return static_cast<int>(f + 0.5f);
	}

	// Clear Vertex Caches!
	void ClearCache();

	void ClearTile();

	void ClearTileMT();

	void ReleaseResource();

	// Compute Varyings Gradient Along Axis X&Y.
	void ComputeGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, toy::vec2 *g);

private:
	Arti3DIndexBuffer		*m_pIndexBuffer;
	Arti3DVertexBuffer		*m_pVertexBuffer;
	Arti3DVertexLayout		*m_pVertexLayout;

	uint32_t				m_iThreadNum;
	uint32_t				m_iWorkingThreadNum;

	Arti3DVertexCache		**m_ppVertexCache;
	Arti3DTransformedVertex	**m_ppTransformedVertex;
	Arti3DTransformedFace	**m_ppTransformedFace;


	Arti3DVertexCache					vCache[g_ciCacheSize];

	Toy_VertexBuffer			vBuffer;
	Toy_IndiceBuffer			iBuffer;

	Arti3DTransformedVertex		tvBuffer[g_ciVertexArraySize];

	std::vector<Arti3DTransformedFace>	faceBuffer;

	std::vector<Arti3D_Tile>		m_aTile;

	Arti3D_Tile					*m_pTiles;

	int							m_TileXCount;
	int							m_TileYCount;

	Toy_VertexBuffer	tvb;

	std::ofstream		dFile;

	RenderTarget		mRT;
	// Render State
	RenderContext	mRC;
};