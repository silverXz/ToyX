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

	// Create A New Thread.
	// @param pParent : Parent Who Created This Thread.
	// @param iThread : Thread ID [0,g_ciMaxThreadNum).
	Arti3DResult Create(Arti3DDevice *pParent,uint32_t iThread);

public:
	// Clear All The Local Caches And Buffers: Vertex Cache, Transformed Face Buffer.
	void ClearCacheAndBuffer();

	// Distribute Work For Every Thread.
	// @param iThread : Thread ID [0,g_ciMaxThreadNum).
	// @param iStart  : Start Index Of The Global Index Buffer.
	// @param iEnd    : End Index Of The Global Index Buffer (Exclusive).
	void DistributeWorkLoad(uint32_t iThread, uint32_t iStart, uint32_t iEnd);
	
	// Thread Function.
	// @param pThread : Pointer To Specific Instance Of Thread.
	static void WorkFunc(Arti3DThread *pThread);

private:
	
	// 1.Fetch Vertex Info. From Vertex Cache.
	// 2.If Not Found In Cache, Execute Vertex Shader For This Vertex.
	// 3.Clip And Assembly Triangle.
	void ProcessVertex();
	
	// 1.Test transformed faces against tiles.
	// 2.Add overlapped face index to tile's local buffer. 
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
	
	// Process Every Fragment Of This Tile. By Processing, We Mean Color Calculation, Depth Test, etc.
	// @param i_pTile : Pointer To Target Tile.
	void RenderFragment(Arti3DTile *i_pTile);
	
	// There Are 3 Types Of Fragment:
	// Type #1 : It Has The Size Of A Tile And It Is Totally Covered By A Triangle.
	// Type #2 : It Has The Size Of A Block And It Is Totally Covered By A Triangle.
	// Type #3 : It Has 4 Consecutive Pixels In A Row With A Coverage Mask.
	
	// Render Type #1 Fragment.
	// @param i_pFrag : Pointer To Target Fragment.
	void RenderTileFragments(Arti3DFragment *i_pFrag);
	
	// Render Type #2 Fragment.
	// @param i_pFrag : Pointer To Target Fragment.
	void RenderBlockFragments(Arti3DFragment *i_pFrag);
	
	// Render Type #3 Fragment.
	// @param i_pFrag : Pointer To Target Fragment.
	void RenderMaskedFragments(Arti3DFragment *i_pFrag);
	
	// Compute Attribute Gradient Along X And Y For A Given Triangle.
	inline void ComputeTriangleGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, a3d::vec2 *o_pVec2);

	inline void CalcVaryings(Arti3DTransformedFace* f, int x, int y, __m128 &W0, __m128 &W1, __m128 &WDY, __m128 *V0, __m128 *V1, __m128 *VDY);
	
	inline void PreInterpolateVaryings(__m128 &W, __m128 *iV, SSE_Float *oV);
	
	inline void IncVaryingsAlongY(__m128 &W0, __m128 &W1, __m128 WDY, __m128 *V0, __m128 *V1, __m128 *VDY);
	
	inline __m128i ConvertColorFormat(SSE_Color3 &src);

private:
	Arti3DVertexCache			*m_pVertexCache;		// Vertex Cache
	Arti3DTransformedFace		*m_pTransformedFace;	// Local Buffer For Transformed Face.
	
	uint32_t					m_iTransformedFace;		// Size Of Local Buffer.
	uint32_t					m_iThread;				// Thread Index.
	uint32_t					m_iStart;				// Work Load Start Index.
	uint32_t					m_iEnd;					// Work Load End Index.

	Arti3DDevice				*m_pParent;				// Parent Who Created This Thread.
};


#endif
