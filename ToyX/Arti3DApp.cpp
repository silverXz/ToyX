#include "stdafx.h"
#include "Arti3DApp.h"

#include "Arti3D_Window.h"
#include "Arti3D_Device.h"
#include "Arti3D_RenderTarget.h"
#include "Arti3D_VertexLayout.h"
#include "Arti3D_VertexBuffer.h"
#include "Arti3D_IndexBuffer.h"
#include "Arti3D_Surface.h"
#include "Clock.h"


Arti3DApp::Arti3DApp() : m_pDevice(nullptr)
, m_pWindow(nullptr)
, m_bRunning(true)
{

}

Arti3DApp::~Arti3DApp()
{
	if (m_pWindow)
		delete m_pWindow;
	if (m_pDevice)
		delete m_pDevice;

	SDL_Quit();
}

void Arti3DApp::Run()
{
	while (m_bRunning)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			HandleEvent(event, this);

		RenderScene();
		CalculateFPS();

		m_pWindow->UpdateSurface();
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
}

Arti3DResult Arti3DApp::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf_s(stderr, "SDL_Init Failed!\n");
		return ARTI3D_UNKOWN;
	}

	Arti3DResult a3dr = CreateArti3DWindow(&m_pWindow,
		"ToyX", 
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		800, 600, 
		SDL_WINDOW_SHOWN);

	if (a3dr != ARTI3D_OK)
		return ARTI3D_UNKOWN;

	Arti3DDeviceParameter a3dDeviceParameter;
	a3dDeviceParameter.bMultiThread = true;
	a3dDeviceParameter.iWidth = 800;
	a3dDeviceParameter.iHeight = 600;

	a3dr = CreateAndInitializeDevice(&m_pDevice, &a3dDeviceParameter);

	return a3dr;
}

void Arti3DApp::RenderScene()
{
	if (!m_pDevice)
		return;
	
	const float rotSpeed = a3d::PI / 20.0f;

	static float rotAngle = 0.0f;
	static double tLast = iv::Clock::GetCurrentTimeMS();


	double curTime = iv::Clock::GetCurrentTimeMS();

	double dt = (curTime - tLast) * 0.001;
	rotAngle += rotSpeed * static_cast<float>(dt);
	if (rotAngle > a3d::TWOPI)
		rotAngle -= a3d::TWOPI;

	m_pDevice->SetMatrix(ARTI3D_MATRIX_MODEL, a3d::rotate(rotAngle, a3d::vec3(0.0f, 1.0f, 0.0f)));

	tLast = curTime;

	m_pDevice->Begin();
	m_pDevice->ClearColorBuffer(a3d::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	m_pDevice->ClearDepthBuffer();
	m_pDevice->DrawMesh_MT();
	m_pDevice->End();
}

Arti3DResult Arti3DApp::CreateArti3DWindow(Arti3DWindow** o_pWindow, const char *pTitle, int x, int y, int width, int height, int flag)
{
	if (!o_pWindow)
		return ARTI3D_INVALID_PARAMETER;

	*o_pWindow = new Arti3DWindow();
	(*o_pWindow)->Create(pTitle, x, y, width, height, flag);

	m_pWindow = *o_pWindow;

	return ARTI3D_OK;
}

Arti3DResult Arti3DApp::CreateAndInitializeDevice(Arti3DDevice **o_pDevice, Arti3DDeviceParameter *pA3DDeviceParameters)
{
	if (!o_pDevice)
		return ARTI3D_INVALID_PARAMETER;

	*o_pDevice = new Arti3DDevice;

	if (!*o_pDevice)
		return ARTI3D_OUT_OF_MEMORY;

	m_pDevice = *o_pDevice;

	m_pDevice->InitializeDevice(*pA3DDeviceParameters);
	
	Arti3DRenderTarget* pRenderTarget = nullptr;
	m_pDevice->CreateRenderTarget(&pRenderTarget);

	Arti3DSurface *pBackbuffer = nullptr;
	m_pDevice->CreateSurfaceFromWindow(&pBackbuffer, m_pWindow);

	Arti3DSurface *pZBuffer = nullptr;
	m_pDevice->CreateRGBSurface(&pZBuffer, m_pWindow->m_iWidth, m_pWindow->m_iHeight, 32, 0, 0, 0, 0);

	pRenderTarget->SetBackBuffer(pBackbuffer);
	pRenderTarget->SetZBuffer(pZBuffer);

	m_pDevice->SetRenderTarget(pRenderTarget);

	return ARTI3D_OK;
}

void Arti3DApp::HandleEvent(const SDL_Event& event, Arti3DApp *pApp)
{
	switch (event.type)
	{
	case SDL_QUIT:
		pApp->m_bRunning = false;
		break;
	case SDL_KEYDOWN:
		HandleKeyEvent(event, pApp);
		break;
	default:
		break;
	}
}

void Arti3DApp::HandleKeyEvent(const SDL_Event& event, Arti3DApp* pApp)
{
	switch (event.key.keysym.sym)
	{
	case SDLK_ESCAPE:
		pApp->m_bRunning = false;
		break;
	default:
		break;
	}
}

void Arti3DApp::SetupScene()
{
	if (!m_pDevice)
		return;

	// Load Obj.
	const float len = 2.0f;

	Arti3DVertexLayout *pVertexLayout = nullptr;
	Arti3DVertexAttributeFormat vaf[] = { ARTI3D_VAF_VECTOR4, ARTI3D_VAF_VECTOR4 };
	m_pDevice->CreateVertexLayout(&pVertexLayout, 2, vaf);
	m_pDevice->SetVertexLayout(pVertexLayout);

	Arti3DVertexBuffer *pVertexBuffer = nullptr;

	uint32_t iFloat = pVertexLayout->iGetFloats();
	uint32_t iStride = iFloat * sizeof(float);
	const uint32_t iVertex = 8;
	m_pDevice->CreateVertexBuffer(&pVertexBuffer, iVertex * iStride);

	// Upload Cube Data To VertexBuffer
	std::vector<std::vector<float>> xv{
		{ -len, len, len, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },	//0
		{ len, len, len, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },	//1
		{ len, len, -len, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f },	//2
		{ -len, len, -len, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f },	//3
		{ -len, -len, len, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f },	//4
		{ len, -len, len, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },	//5
		{ len, -len, -len, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },	//6
		{ -len, -len, -len, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }	//7
	};

	for (int i = 0; i < iVertex; ++i)
	{
		void *pDest = nullptr;
		pVertexBuffer->GetPointer(i * iStride, &pDest);
		memcpy(pDest, &xv[i][0], iStride);
	}

	m_pDevice->SetVertexBuffer(pVertexBuffer);

	Arti3DIndexBuffer *pIndexBuffer = nullptr;
	m_pDevice->CreateIndexBuffer(&pIndexBuffer, 36 * sizeof(uint32_t), ARTI3D_FORMAT_INDEX32);
	uint32_t xid[] = { 0, 1, 2, 0, 2, 3, 0, 4, 5, 0, 5, 1, 1, 5, 6, 1, 6, 2, 4, 7, 6, 4, 6, 5, 0, 3, 7, 0, 7, 4, 3, 2, 6, 3, 6, 7 };
	void *pDest = nullptr;
	pIndexBuffer->GetPointer(0, &pDest);
	memcpy(pDest, xid, sizeof(xid));

	m_pDevice->SetIndexBuffer(pIndexBuffer);

	m_pDevice->SetMatrix(ARTI3D_MATRIX_VIEW, a3d::lookAt(a3d::vec3(4.0f, 4.0f, 4.0f), a3d::vec3(0.0f, 0.0f, 0.0f), a3d::vec3(0.0f, 1.0f, 0.0f)));
	m_pDevice->SetMatrix(ARTI3D_MATRIX_PROJECTION, a3d::perspective(90.0f, (float)m_pWindow->m_iWidth / m_pWindow->m_iHeight, 0.1f, 15.0f));
	m_pDevice->SetViewport(0, 0, m_pWindow->m_iWidth, m_pWindow->m_iHeight);

	m_pDevice->SetVertexShader(new SimpleCubeVS(m_pDevice));
	m_pDevice->SetPixelShader(new SimpleCubePS(m_pDevice));

	CreateCheckboardTexture();
	
	m_pDevice->InitializeWorkThreads();
}

void Arti3DApp::CalculateFPS()
{
	static double tLast = iv::Clock::GetCurrentTimeMS();
	static int iFrames = 0;
	 
	double tNow = iv::Clock::GetCurrentTimeMS();
	if (tNow - tLast >= 1000.0)
	{
		std::cout << "FPS:" << iFrames << std::endl;
		iFrames = 0;
		tLast = tNow;
	}
	else
		++iFrames;
}

void Arti3DApp::CreateCheckboardTexture()
{
	if (!m_pDevice)
		return;
	int iTexWidth = 320;
	int iTexHeight = 320;

	PArti3DSurface pSurface = nullptr;

	Arti3DResult a3dr = m_pDevice->CreateRGBSurface(&pSurface, iTexWidth, iTexHeight, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	
	if (a3dr != ARTI3D_OK)
		return;

	int iBoardLength = 32;

	uint32_t white = 0xFFFFFFFF;
	uint32_t black = 0x0;

	uint32_t cc = white;

	uint32_t *pp = (uint32_t*)pSurface->pGetPixelsDataPtr();

	for (int i = 0; i < iTexHeight; ++i)
	{
		int br = (i & iBoardLength);
		for (int j = 0; j < iTexWidth; ++j)
		{
			cc = (j & iBoardLength) ^ br ? black : white;
			pp[i * iTexWidth + j] = cc;
		}
	}

	m_pDevice->AttachTextureUnit(pSurface, 0);
}

void Arti3DApp::SetupScene2()
{
	if (!m_pDevice)
		return;
	
	// Load Obj.
	const float len = 2.0f;

	Arti3DVertexLayout *pVertexLayout = nullptr;
	Arti3DVertexAttributeFormat vaf[] = { ARTI3D_VAF_VECTOR4, ARTI3D_VAF_VECTOR2 };
	m_pDevice->CreateVertexLayout(&pVertexLayout, 2, vaf);
	m_pDevice->SetVertexLayout(pVertexLayout);

	Arti3DVertexBuffer *pVertexBuffer = nullptr;

	uint32_t iFloat = pVertexLayout->iGetFloats();
	uint32_t iStride = iFloat * sizeof(float);

	const uint32_t iVertex = 24;
	m_pDevice->CreateVertexBuffer(&pVertexBuffer, iVertex * iStride);

	// Upload Cube Data To VertexBuffer
	std::vector<std::vector<float>> xv{
		{ -len, len, len, 1.0f, 0.0f, 1.0f},	//0,0 - front
		{ -len, -len,len, 1.0f, 0.0f, 0.0f},	//1,4 - front
		{ len, -len, len, 1.0f, 1.0f, 0.0f},	//2,5 - front
		{ len, len,  len, 1.0f, 1.0f, 1.0f},    //3,1 - front

		{ len, len,  len, 1.0f, 0.0f, 1.0f},	//4,1 - right
		{ len,-len,  len, 1.0f, 0.0f, 0.0f},	//5,5 - right
		{ len, -len,-len, 1.0f, 1.0f, 0.0f},	//6,6 - right
		{ len, len, -len, 1.0f, 1.0f, 1.0f},	//7,2 - right

		{ len, len, -len, 1.0f, 0.0f, 1.0f},	//8,2 - back
		{ len,-len, -len, 1.0f, 0.0f, 0.0f},	//9,6 - back
		{ -len,-len,-len, 1.0f, 1.0f, 0.0f},	//10,7 - back
		{ -len,len, -len, 1.0f, 1.0f, 1.0f},	//11,3 - back

		{ -len, len,-len, 1.0f, 0.0f, 1.0f},	//12,3 - right
		{ -len,-len,-len, 1.0f, 0.0f, 0.0f},	//13,7 - right
		{ -len,-len, len, 1.0f, 1.0f, 0.0f},	//14,4 - right
		{ -len, len, len, 1.0f, 1.0f, 1.0f},	//15,0 - right

		{ -len, len,-len, 1.0f, 0.0f, 1.0f},	//16,3 - up
		{ -len, len, len, 1.0f, 0.0f, 0.0f},	//17,0 - up
		{ len, len,  len, 1.0f, 1.0f, 0.0f},	//18,1 - up
		{ len, len, -len, 1.0f, 1.0f, 1.0f},	//19,2 - up

		{ -len,-len, len, 1.0f, 0.0f, 1.0f},	//20,4 - down
		{ -len,-len,-len, 1.0f, 0.0f, 0.0f},	//21,7 - down
		{ len,-len, -len, 1.0f, 1.0f, 0.0f},	//22,6 - down
		{ len, -len, len, 1.0f, 1.0f, 1.0f}		//23,5 - down
	};

	for (int i = 0; i < iVertex; ++i)
	{
		void *pDest = nullptr;
		pVertexBuffer->GetPointer(i * iStride, &pDest);
		memcpy(pDest, &xv[i][0], iStride);
	}

	m_pDevice->SetVertexBuffer(pVertexBuffer);

	Arti3DIndexBuffer *pIndexBuffer = nullptr;
	m_pDevice->CreateIndexBuffer(&pIndexBuffer, 36 * sizeof(uint32_t), ARTI3D_FORMAT_INDEX32);
	uint32_t xid[] = { 0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23};

	void *pDest = nullptr;
	pIndexBuffer->GetPointer(0, &pDest);
	memcpy(pDest, xid, sizeof(xid));

	m_pDevice->SetIndexBuffer(pIndexBuffer);

	m_pDevice->SetMatrix(ARTI3D_MATRIX_VIEW, a3d::lookAt(a3d::vec3(4.0f, 4.0f, 4.0f), a3d::vec3(0.0f, 0.0f, 0.0f), a3d::vec3(0.0f, 1.0f, 0.0f)));
	m_pDevice->SetMatrix(ARTI3D_MATRIX_PROJECTION, a3d::perspective(90.0f, (float)m_pWindow->m_iWidth / m_pWindow->m_iHeight, 0.1f, 15.0f));
	m_pDevice->SetViewport(0, 0, m_pWindow->m_iWidth, m_pWindow->m_iHeight);

	m_pDevice->SetVertexShader(new CheckboardCubeVS(m_pDevice));
	m_pDevice->SetPixelShader(new CheckboardCubePS(m_pDevice));

	CreateCheckboardTexture();

	m_pDevice->InitializeWorkThreads();
}
