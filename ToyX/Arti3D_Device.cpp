#include "stdafx.h"
#include "Arti3D_Device.h"
#include <algorithm>
#include <cstdlib>
#include <assert.h>

#include <xmmintrin.h>
#include <smmintrin.h>
#include <emmintrin.h>

#include <SDL/SDL.h>

#include "Arti3DApp.h"
#include "Arti3D_RenderTarget.h"
#include "Arti3D_VertexBuffer.h"
#include "Arti3D_IndexBuffer.h"
#include "Arti3D_Thread.h"
#include "Arti3D_Tile.h"
#include "Arti3D_Surface.h"
#include "Arti3D_ShaderBase.h"


using namespace a3d;

Arti3DDevice::Arti3DDevice() : m_pIndexBuffer(nullptr),
	m_pVertexBuffer(nullptr),
	m_pTiles(nullptr),
	m_pRenderTarget(nullptr),
	m_pJobQueue(nullptr),
	m_pThreads(nullptr),
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

	for (int i = 0; i < ARTI3D_MAX_TEXTURE_UNIT; ++i)
		SAFE_DELETE(mRC.globals.pSurfaces[i]);

	SAFE_DELETE(m_pRenderTarget);
	SAFE_DELETE_ARRAY(m_pThreads);
	SAFE_DELETE_ARRAY(m_pTiles);
	SAFE_DELETE_ARRAY(m_pJobQueue);
}

Arti3DResult Arti3DDevice::InitializeDevice(Arti3DDeviceParameter deviceParam)
{

	m_iWidth = deviceParam.iWidth;
	m_iHeight = deviceParam.iHeight;

	if (deviceParam.bMultiThread)
		CreateWorkerThreads();

	CreateTilesAndJobQueue();

	memset(&mRC, 0, sizeof(RenderContext));

	return ARTI3D_OK;
}

void Arti3DDevice::ClearColorBuffer(const a3d::vec4& color)
{
	Arti3DSurface *pbb = m_pRenderTarget ? m_pRenderTarget->m_pBackbuffer : nullptr;

	if (!pbb)
		return;
	int hr = SDL_FillRect(pbb->m_pSurface, nullptr, CvrtToUint32(color));

	if (hr)
		std::cerr << "Failed to clear color buffer!\n";
}

void Arti3DDevice::ClearDepthBuffer(float cDepth)
{
	Arti3DSurface *pzb = m_pRenderTarget ? m_pRenderTarget->m_pZBuffer : nullptr;
	if (!pzb)
		return;

	SDL_FillRect(pzb->m_pSurface, nullptr, 0);
}

void Arti3DDevice::SetMatrix(Arti3DMatrixType matrixType, const a3d::mat4& m)
{
	Arti3DShaderUniform &rgu = mRC.globals;
	switch (matrixType)
	{
	case ARTI3D_MATRIX_MODEL:
		rgu.model = m;
		break;
	case ARTI3D_MATRIX_VIEW:
		rgu.view = m;
		break;
	case ARTI3D_MATRIX_PROJECTION:
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

void Arti3DDevice::SetPixelColor(int x, int y, uint32_t c)
{
	Arti3DSurface *pbb = m_pRenderTarget ? m_pRenderTarget->pGetBackBuffer() : nullptr;

	if (!pbb || x >= pbb->iGetWidth() || y >= pbb->iGetHeight())	
		return;

	uint8_t *pTargetRow = reinterpret_cast<uint8_t*>(pbb->pGetPixelsDataPtr()) + pbb->iGetPitch();

	((uint32_t*)pTargetRow)[x] = c;
}

void Arti3DDevice::Draw3DSolidTriangle(const a3d::vec4& p1, const a3d::vec4& p2, const a3d::vec4& p3, const a3d::vec4& c)
{
	Arti3DSurface *pbb = m_pRenderTarget ? m_pRenderTarget->m_pBackbuffer : nullptr;

	if (!pbb)
		return;

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
	
	int w = pbb->iGetWidth();
	int h = pbb->iGetHeight();

	float half_width = 0.5f * w;
	float half_height = 0.5f * h;

	clip1.x = (clip1.x + 1.0f) * half_width;
	clip1.y = h - (clip1.y + 1.0f) * half_height;
	clip2.x = (clip2.x + 1.0f) * half_width;
	clip2.y = h - (clip2.y + 1.0f) * half_height;
	clip3.x = (clip3.x + 1.0f) * half_width;
	clip3.y = h - (clip3.y + 1.0f) * half_height;	
}

void Arti3DDevice::Begin()
{
	Arti3DSurface *pbb = m_pRenderTarget ? m_pRenderTarget->m_pBackbuffer : nullptr;

	if (!pbb)
	{
		std::cerr << "ToyRender::Begin(): No Backbuffer!\n";
		return;
	}

	pbb->Lock();

}

void Arti3DDevice::End()
{
	Arti3DSurface *pbb = m_pRenderTarget ? m_pRenderTarget->m_pBackbuffer : nullptr;

	if (!pbb)
	{
		std::cerr << "ToyRender::End(): No Backbuffer!\n";
		return;
	}

	pbb->UnLock();
}

void Arti3DDevice::SetRenderTarget(Arti3DRenderTarget *pRenderTarget)
{
	m_pRenderTarget = pRenderTarget;
}


void Arti3DDevice::DrawTileGrid()
{
	Arti3DSurface *pbb = m_pRenderTarget->m_pBackbuffer;

	int iWidth = pbb->iGetWidth();
	int iHeight = pbb->iGetHeight();

	uint32_t gridColor = CvrtToUint32(a3d::vec4(1.0f,1.0f,1.0f,1.0f));

	//Draw Vertical Lines
	for (int x = 0; x < iWidth; x += g_ciTileSize)
		Draw2DLines(x, 0, x, iHeight - 1, gridColor);
	//Draw Horizontal Lines
	for (int y = 0; y < iHeight; y += g_ciTileSize)
		Draw2DLines(0, y, iWidth - 1, y, gridColor);
}

Arti3DResult Arti3DDevice::CreateTilesAndJobQueue()
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
			// Calculate top-left corner for this tile.
			uint32_t iX = x * g_ciTileSize;
			uint32_t iY = y * g_ciTileSize;

			uint32_t iWidth, iHeight;
			
			Arti3DTile *pTile = &m_pTiles[y * m_iTileX + x];
			
			if (x == m_iTileX - 1)
			{
				int iExtraX = m_iWidth & (g_ciTileSize - 1);
				iWidth = iExtraX ? iExtraX : g_ciTileSize;
			}
			else
				iWidth = g_ciTileSize;

			if (y == m_iTileY - 1)
			{
				int iExtraY = m_iHeight & (g_ciTileSize - 1);
				iHeight = iExtraY ? iExtraY : g_ciTileSize;
			}
			else
				iHeight = g_ciTileSize;

			pTile->Create(iX, iY, iWidth, iHeight);
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

Arti3DResult Arti3DDevice::CreateWorkerThreads()
{
	m_pThreads = new Arti3DThread[ARTI3D_MAX_THREAD];

	for (int i = 0; i < ARTI3D_MAX_THREAD; ++i)
		m_pThreads[i].Create(this, i);
	
	m_iStage = 1;

	for (int i = 0; i < ARTI3D_MAX_THREAD; ++i)
		m_vThread.push_back(std::thread(Arti3DThread::WorkFunc, &m_pThreads[i]));

	return ARTI3D_OK;
}


Arti3DResult Arti3DDevice::CreateRenderTarget(Arti3DRenderTarget **o_pRenderTarget)
{
	if (!o_pRenderTarget)
		return ARTI3D_INVALID_PARAMETER;

	*o_pRenderTarget = new Arti3DRenderTarget(this);

	if (!*o_pRenderTarget)
		return ARTI3D_OUT_OF_MEMORY;
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

Arti3DResult Arti3DDevice::CreateVertexBuffer(Arti3DVertexBuffer **o_pVertexBuffer, Arti3DVertexLayout* o_pVertexLayout, uint32_t iVertexCount)
{
	if (!o_pVertexBuffer || iVertexCount == 0)
		return ARTI3D_INVALID_PARAMETER;

	*o_pVertexBuffer = new Arti3DVertexBuffer();

	if (!*o_pVertexBuffer)
		return ARTI3D_OUT_OF_MEMORY;

	(*o_pVertexBuffer)->Create(o_pVertexLayout, iVertexCount);

	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::BindVertexBuffer(Arti3DVertexBuffer *pVertexBuffer)
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

Arti3DResult Arti3DDevice::BindIndexBuffer(Arti3DIndexBuffer *pIndexBuffer)
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
	m_iWorkingThread = ARTI3D_MAX_THREAD;
	
	// It's Important That The Following Line Comes The Last.
	m_iStage = 0;
	
	SyncronizeWorkerThreads();
	ClearTilesAndJobQueue();
}

Arti3DResult Arti3DDevice::DistributeThreadWorkload()
{
	// Allocate Work Load For Every Threads.
	uint32_t iNumTriangle = m_pIndexBuffer->iGetIndexNum() / 3;
	if (m_pIndexBuffer->iGetIndexNum() % 3 != 0)
		return ARTI3D_INVALID_BUFFER_SIZE;

	uint32_t iFacePerThread = iNumTriangle / ARTI3D_MAX_THREAD;
	uint32_t iExtraFace = iNumTriangle % ARTI3D_MAX_THREAD;

	uint32_t workLoad[ARTI3D_MAX_THREAD];
	
	for (auto& x : workLoad)
		x = iFacePerThread;

	if (iExtraFace)
	{
		for (uint32_t i = 0; i < iExtraFace; ++i)
			++workLoad[i];
	}

	uint32_t iStart = 0, iEnd = 0;

	for (uint32_t i = 0; i < ARTI3D_MAX_THREAD; ++i)
	{
		iEnd = iStart + workLoad[i] * 3 - 1;
		m_pThreads[i].DistributeWorkLoad(i, iStart, iEnd);
		iStart = iEnd + 1;
	}

	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::ClearTilesAndJobQueue()
{
	// Clear The Tile.
	uint32_t iTileNum = m_iTileX * m_iTileY;

	for (uint32_t i = 0; i < iTileNum; ++i)
		m_pTiles[i].Clear();

	// Clear The Job Queue.
	m_iJobStart = m_iJobStart2 = m_iJobEnd = 0;
	return ARTI3D_OK;
}

void Arti3DDevice::SyncronizeWorkerThreads()
{
	while (!(m_iStage == 1 && m_iWorkingThread == 0))
		std::this_thread::yield();
}

Arti3DFormat Arti3DDevice::MasksToPixelsFormatEnum(int bpp, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask)
{
	switch (bpp)
	{
	case 8:
		if (Rmask == 0)
			return ARTI3D_FORMAT_INDEX8;
		break;
	case 16:
		if (Rmask == 0)
			return ARTI3D_FORMAT_INDEX16;
		if (Rmask == 0xF0 && Gmask == 0x0F)
			return ARTI3D_FORMAT_RG88;
		break;
	case 24:
		if (Rmask == 0x0F00 && Gmask == 0x00F0 && Bmask == 0x000F)
			return ARTI3D_FORMAT_RGB888;
		break;
	case 32:
		if (Rmask == 0xFFFF)
			return ARTI3D_FORMAT_R32F;
		if (Rmask == 0xF000 && Gmask == 0x0F00 && Bmask == 0x00F0 && Amask == 0x000F)
			return ARTI3D_FORMAT_RGBA8888;
		if (Rmask == 0 && Gmask == 0 && Bmask == 0 && Amask == 0)
			return ARTI3D_FORMAT_INDEX32;
		break;
	default:
		break;
	}
	return ARTI3D_FORMAT_INVLAID;
}

Arti3DResult Arti3DDevice::CreateRGBSurface(Arti3DSurface **o_pSurface,uint32_t width,uint32_t height,uint32_t bpp, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask)
{
	if (!o_pSurface)
		return ARTI3D_INVALID_PARAMETER;
	*o_pSurface = new Arti3DSurface(this);
	(*o_pSurface)->Create(bpp, width, height, rmask, gmask, bmask, amask);
	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::CreateRGBSurface(Arti3DSurface **o_pSurface, const char *pFilepath)
{
	if (!o_pSurface)
		return ARTI3D_INVALID_PARAMETER;
	*o_pSurface = new Arti3DSurface(this);
	(*o_pSurface)->Create(pFilepath);
	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::CreateSurfaceFromWindow(Arti3DSurface **o_pSurface,Arti3DWindow *pWindow)
{
	if (!o_pSurface)
		return ARTI3D_INVALID_PARAMETER;
	*o_pSurface = new Arti3DSurface(this);
	(*o_pSurface)->Create(pWindow);
	return ARTI3D_OK;
}

Arti3DResult Arti3DDevice::AttachTextureUnit(PArti3DSurface pSurface, int iTexUint)
{
	if ( !pSurface || iTexUint >= ARTI3D_MAX_TEXTURE_UNIT || iTexUint < 0)
		return ARTI3D_INVALID_PARAMETER;

	mRC.globals.pSurfaces[iTexUint] = pSurface;

	return ARTI3D_OK;
}

