#pragma once

#include <stdint.h>
#include <SDL/SDL.h>
#include <vector>
#include <fstream>
#include "ToyMath.h"
#include "Shader.h"
#include "ToyTypes.h"





class ToyRender
{

public:



public:
	ToyRender();
	~ToyRender();


	void InitTile();


	void SetRenderTarget(const RenderTarget& rRT);


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


	void SetVertexShader(VertexShader vs);
	void SetFragmentShader(FragmentShader fs);

	// Render State Related
	void SetMatrix(MatrixType matrixType, const toy::mat4& m);
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
	void RasterizeTriangle_SIMD(Toy_TransformedFace *f);

	void Tilize(uint32_t faceid);

	void ProcessV_WithClip();
	
	// Process Vertex!
	void ProcessV();
	void TransformVertex(uint32_t in, Toy_TransformedVertex *out);
	void PostProcessV(Toy_TransformedVertex *v);

	// Clipping Functions!
	inline int CalcClipMask(Toy_TransformedVertex *v);
	void ClipTriangle(Toy_TransformedVertex *v1, Toy_TransformedVertex *v2, Toy_TransformedVertex *v3);
	void InsertTransformedFace(Toy_TransformedVertex *v1, Toy_TransformedVertex *v2, Toy_TransformedVertex *v3);

	// Process Rasterization!
	void ProcessR();

	void RasterizeTile();

	void RenderFragments();

	void RenderTileFragments(Toy_Fragment *frag);
	void RenderBlockFragments(Toy_Fragment *frag);
	void RenderMaskedFragments(Toy_Fragment *frag);

	inline void CalcVaryings(Toy_TransformedFace* f,int x,int y,__m128 &W0,__m128 &W1,__m128 &WDY,__m128 *V0,__m128 *V1,__m128 *VDY);
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

	// Compute Varyings Gradient Along Axis X&Y.
	void ComputeGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, toy::vec2 *g);

private:

	VertexCache					vCache[CACHE_SIZE];

	Toy_VertexBuffer			vBuffer;
	Toy_IndiceBuffer			iBuffer;

	Toy_TransformedVertex		tvBuffer[VERTEX_ARRAY_SIZE];

	std::vector<Toy_TransformedFace>	faceBuffer;

	std::vector<Toy_Tile>		m_aTile;
	int							m_TileXCount;
	int							m_TileYCount;

	Toy_VertexBuffer	tvb;

	std::ofstream		dFile;

	RenderTarget		mRT;
	// Render State
	RenderContext	mRC;
};