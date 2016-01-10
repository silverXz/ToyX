#pragma once

#include <stdint.h>
#include <SDL/SDL.h>
#include <vector>
#include <fstream>
#include <atomic>
#include <thread>
#include "Arti3D_Types.h"
#include "Arti3D_ForwardDecl.h"
#include "Arti3D_RenderTarget.h"

struct Arti3DDeviceParameter
{
	uint32_t	iWidth, iHeight;
	bool		bMultiThread;
};

struct RenderContext
{
	PArti3DVertexShader	pVertexShader;
	PArti3DPixelShader	pPixelShader;
	Arti3DShaderUniform	globals;
	PArti3DSurface		pSurfaces[ARTI3D_MAX_TEXTURE_UNIT];
};

class Arti3DDevice
{
	friend class Arti3DThread;
	friend class Arti3DPixelShader;
	friend class Arti3DApp;

public:
	Arti3DDevice();
	~Arti3DDevice();

	Arti3DResult InitializeDevice(Arti3DDeviceParameter deviceParam);

	Arti3DResult PreRender();
	
	void Begin();

	void End();

	// Draw Calls

	// Bresenham algorithm
	void Draw2DLines(int x1, int y1, int x2, int y2, uint32_t color);
	
	void Draw3DLines(const a3d::vec4& p1, const a3d::vec4 p2, uint32_t color);

	inline void SetPixelColor(int x, int y, uint32_t c);


	// Get pixel format based on channel masks
	// @param bpp : bits per pixel.
	// @param Rmask : red channel mask.
	// @param Gmask : green channel mask.
	// @param Bmask : blue channel mask.
	// @param Amask : alpha channel mask.
	Arti3DFormat MasksToPixelsFormatEnum(int bpp, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);

	// Create g_ciMaxThreadNum Threads And Execute The Thread Function "WorkFunc".
	Arti3DResult CreateWorkerThreads();

	// Distribute Work For Every Threads.
	// This Function Must Be Called After The Scene Is Loaded And The Index Buffer Is Properly Setup.
	Arti3DResult DistributeThreadWorkload();

	// Create Render Target
	Arti3DResult CreateRenderTarget(Arti3DRenderTarget **o_pRenderTarget);

	void SetRenderTarget(Arti3DRenderTarget *pRenderTarget);


	// Create RGB Surface For Texture/Depth
	// @param o_pSurface : pointer to pointer to Arti3DSurface.
	// @param width	: width of surface.
	// @param height : height of surface.
	// @param bpp : bits per pixel.
	// @param rmask : red channel mask.
	// @param gmask : green channel mask.
	// @param bmask : blue channel mask.
	// @param amask : alpha channel mask.
	Arti3DResult CreateRGBSurface(Arti3DSurface **o_pSurface,uint32_t width,uint32_t height,uint32_t bpp, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask);

	// Create Surface Associated With Some Windows For Backbuffer.
	// @param pWindow : pointer to the window that the surface associated with.
	Arti3DResult CreateSurfaceFromWindow(Arti3DSurface **o_pSurface,Arti3DWindow *pWindow);

	// Create A VertexLayout Instance To Specify The Attibute Data Layout Of A Vertex.
	// @param o_pVertexLayout : Pointer To The Target Arti3DVertexLayout Pointer.
	// @param iAttribute : The Number Of Attributes A Vertex Has.
	// @param i_pVAFormat : Pointer To An Array Of Arti3DVertexAttributeFormat To Specify The Format Of Every Vertex Attribute. It Should At Least Have "iAttribute" Elements.
	Arti3DResult CreateVertexLayout(Arti3DVertexLayout **o_pVertexLayout, uint32_t iAttribute, Arti3DVertexAttributeFormat *i_pVAFormat);

	// Create Buffer For Triangle Vertices. By Creating, I Mean Allocating Space.
	// @param o_pVertexBuffer : Pointer To The Target Vertex Buffer Pointer.
	// @param o_pVertexLayout : Pointer to Arti3DVertexLayout.
	// @param iVertexCount : The vertex count.
	Arti3DResult CreateVertexBuffer(Arti3DVertexBuffer **o_pVertexBuffer, Arti3DVertexLayout* o_pVertexLayout,uint32_t iVertexCount);

	// Set Current Vertex Buffer.
	// @param pVertexBuffer : Pointer To The Source Vertex Buffer.
	Arti3DResult BindVertexBuffer(Arti3DVertexBuffer *pVertexBuffer);

	// Create Buffer For Triangle Indices. By Creating, I Mean Allocating Space.
	// @param o_pIndexBuffer : Pointer To The Target Index Buffer Pointer.
	// @param iLength : The Storage That The Buffer Needs, In Bytes.
	// @param format : The Format Of Index. ARTI3D_INDEX16 Or ARTI3D_INDEX32.
	Arti3DResult CreateIndexBuffer(Arti3DIndexBuffer **o_pIndexBuffer, uint32_t iLength,Arti3DFormat format);

	// Set Current Index Buffer.
	// @param pIndexBuffer : Pointer To The Source Index Buffer.
	Arti3DResult BindIndexBuffer(Arti3DIndexBuffer *pIndexBuffer);

	Arti3DResult AttachTextureUnit(PArti3DSurface pSurface, int iTexUint);

	void SetVertexShader(PArti3DVertexShader pVertexShader)
	{
		mRC.pVertexShader = pVertexShader;
	}

	void SetPixelShader(PArti3DPixelShader pPixelShader)
	{
		mRC.pPixelShader = pPixelShader;
	}

	// Render State Related
	void SetMatrix(Arti3DMatrixType matrixType, const a3d::mat4& m);
	void SetViewport(int x, int y, int width, int height);

	void ClearColorBuffer(const a3d::vec4& color);
	void ClearDepthBuffer(float cDepth = 0.0f);

	void Draw3DSolidTriangle(const a3d::vec4& p1,const a3d::vec4& p2,const a3d::vec4& p3,const a3d::vec4& c);

	void DrawMesh_MT();

private:
	// Stop All Working Threads. It Is Called In The Dtor Before
	// Thread Objects Are Deleted.
	void StopAllThreads();

	// As The Name Implies, Create The Tiles And Job Queue.
	Arti3DResult CreateTilesAndJobQueue();

	// Clean Up The Job Queue For Next Frame.
	Arti3DResult ClearTilesAndJobQueue();

	// Wait For All Worker Threads Finished Rendering Current Frame.
	void SyncronizeWorkerThreads();

	// For Debug Use.
	void DrawTileGrid();
	
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

	void ReleaseResource();
	
private:
	Arti3DIndexBuffer		*m_pIndexBuffer;
	Arti3DVertexBuffer		*m_pVertexBuffer;

	PArti3DThread			m_pThreads;

	std::vector<std::thread>	m_vThread;

	Arti3DTile					*m_pTiles;

	uint32_t					*m_pJobQueue;
	std::atomic<uint32_t>		m_iJobStart;
	std::atomic<uint32_t>		m_iJobEnd;
	std::atomic<uint32_t>		m_iJobStart2;

	std::atomic<uint32_t>		m_iWorkingThread;

	std::atomic<uint32_t>		m_iStage;

	int							m_iWidth;
	int							m_iHeight;

	int							m_iTileX;
	int							m_iTileY;

	std::ofstream				dFile;

	Arti3DRenderTarget			*m_pRenderTarget;

	// Render State
	RenderContext				mRC;

	bool						m_bThreadStop;
};