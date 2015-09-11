#ifndef _ARTI3D_THREAD_H_
#define _ARTI3D_THREAD_H_

#include "Arti3D_Types.h"

class Arti3DTile;

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
	
	// 1.Fetch Vertex Info. From Vertex Cache.
	// 2.If Not Found In Cache, Execute Vertex Shader For This Vertex.
	// 3.Clip And Assembly Triangle.
	void ProcessVertex();
	
	// 1.Test Triangles In Thread's Local Buffer Against Tiles.
	// 2.Add Overlapped Triangle ID To Tile's Local Buffer. 
	// 3.Then Add The Tile To Arti3DDevice's Job Queue For Further Process (Raterization). 
	void PreProcessTile();

	// 1. Perspective Division.
	// 2. Scale And Transform NDC Coordinate To Screen Space.
	// @param io_pVSOutput : Vertex Shader Output, In Clip Coordinates.
	void PostProcessVertex(Arti3DVSOutput *io_pVSOutput);
	
	// 1.Fetch Result From Vertex Cache First.
	// 2.If Not Found, Excute Vertex Shader For This Vertex.
	// @param i_iVertexIndex : Index Of Target Vertex.
	// @param o_pVSOutput : Pointer To Result.
	void GetTransformedVertex(uint32_t i_iVertexIndex, Arti3DVSOutput *o_pVSOutput);
	
	// 1.Do Triangle Clipping In Clip Coordinates.
	// 2.Interpolate Attributes For Newly Added Vertex If There Is Any.
	// 3.Call PostProcessVertex To.... Post Process The Vertex.
	// @param v1,v2,v3: Pointer To Vertex Shader's Origin Output/Vertex Which Is In Clip Coordinates.
	void ClipTriangle(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3);
	
	// 1.Calculate Triangle Info. e.g.Attribute Gradients Along X,Y.
	// 2.Add Transformed Faces To Thread's Local Buffer.
	// @param v1,v2,v3: Pointer To Post Processed VSOutput/Vertex.
	void AddTransformedFace(Arti3DVSOutput *v1, Arti3DVSOutput *v2, Arti3DVSOutput *v3);
	
	// Rasterize Tile To Generate Fragments For Processing.
	void RasterizeTile(Arti3DTile *io_pTile);

	void RenderFragment(Arti3DTile *i_pTile);

	void RenderTileFragments(Arti3DFragment *i_pFrag);
	void RenderBlockFragments(Arti3DFragment *i_pFrag);
	void RenderMaskedFragments(Arti3DFragment *i_pFrag);
// Compute Attribute Gradient Along X And Y For A Given Triangle.
	inline void ComputeTriangleGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, toy::vec2 *o_pVec2);
	inline void CalcVaryings(Arti3DTransformedFace* f, int x, int y, __m128 &W0, __m128 &W1, __m128 &WDY, __m128 *V0, __m128 *V1, __m128 *VDY);
	inline void PreInterpolateVaryings(__m128 &W, __m128 *iV, SSE_Float *oV);
	inline void IncVaryingsAlongY(__m128 &W0, __m128 &W1, __m128 WDY, __m128 *V0, __m128 *V1, __m128 *VDY);
	inline __m128i ConvertColorFormat(SSE_Color3 &src);

private:
	Arti3DVertexCache		*m_pVertexCache;
	Arti3DTransformedFace	*m_pTransformedFace;
	uint32_t				m_iTransformedFace;
	uint32_t				m_iThread;
	uint32_t				m_iStart, m_iEnd;

	Arti3DDevice			*m_pParent;
};


#endif