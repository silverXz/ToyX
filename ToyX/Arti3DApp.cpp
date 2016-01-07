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
	m_pDevice->SetVertexShader(NewCubeVS);
	m_pDevice->SetPixelShader(NewCubeFS);

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
	int iTexWidth = 800;
	int iTexHeight = 800;
	Arti3DResult a3dr = m_pDevice->CreateRGBSurface(&m_pDevice->m_pTexture, iTexWidth, iTexHeight, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	
	if (a3dr != ARTI3D_OK)
		return;

	int iBoardLength = 32;

	uint32_t white = 0xFFFF;
	uint32_t black = 0x0;

	uint32_t cc = white;

	uint32_t *pp = (uint32_t*)m_pDevice->m_pTexture->pGetPixelsDataPtr();

	for (int i = 0; i < iTexHeight; ++i)
	{
		int br = (i & iBoardLength);
		for (int j = 0; j < iTexWidth; ++j)
		{
			cc = (j & iBoardLength) ^ br ? black : white;
			pp[i * iTexWidth + j] = cc;
		}
	}

	// Let's try to sample.
	__m128 u = _mm_set_ps(0.1f, 0.3f, 0.7f, 0.9f);
	__m128 v = _mm_set_ps(0.2f, 0.7f, 0.5f, 0.6f);

	__m128 tU = _mm_mul_ps(u, _mm_set_ps1(iTexWidth - 1));
	__m128 tV = _mm_mul_ps(v, _mm_set_ps1(iTexHeight - 1));

	__m128i iU = _mm_cvtps_epi32(tU);
	__m128i iV = _mm_cvtps_epi32(tV);

	int ipitch = m_pDevice->m_pTexture->iGetPitch();
	int iBytesPerPixel = m_pDevice->m_pTexture->iGetBitPerPixel() / 8;

	auto mm_mul_epu32 = [](__m128i& a, __m128i& b) {
		__m128i tmp1 = _mm_mul_epi32(a, b);
		__m128i tmp2 = _mm_mul_epi32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4));
		return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1,_MM_SHUFFLE(0,0,2,0)),
			_mm_shuffle_epi32(tmp2,_MM_SHUFFLE(0,0,2,0)));
	};

	__m128i fuck1 = mm_mul_epu32(iV, _mm_set1_epi32(ipitch));
	__m128i fuck2 = mm_mul_epu32(iU, _mm_set1_epi32(iBytesPerPixel));

	__m128i iSamples = _mm_add_epi32(fuck1,fuck2);

	uint8_t *ps = reinterpret_cast<uint8_t*>(pp);

	uint32_t sb = _mm_extract_epi32(iSamples, 0);
	uint32_t color0 = *reinterpret_cast<uint32_t*>(&ps[sb]);
	uint32_t color1 = *reinterpret_cast<uint32_t*>(&ps[_mm_extract_epi32(iSamples, 1)]);
	uint32_t color2 = *reinterpret_cast<uint32_t*>(&ps[_mm_extract_epi32(iSamples, 2)]);
	uint32_t color3 = *reinterpret_cast<uint32_t*>(&ps[_mm_extract_epi32(iSamples, 3)]);

	__m128i color = _mm_set_epi32(color3, color2, color1, color0);
		
}
