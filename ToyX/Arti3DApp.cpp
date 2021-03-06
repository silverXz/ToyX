#include "stdafx.h"
#include "Arti3DApp.h"

#include "Arti3D.h"


Arti3DApp::Arti3DApp() : m_pDevice(nullptr)
, m_pWindow(nullptr)
, m_bRunning(true)
, m_pMesh(nullptr)
, m_pVertexShader(nullptr)
, m_pPixelShader(nullptr)
, m_pBackbuffer(nullptr)
, m_pZBuffer(nullptr)
{

}

Arti3DApp::~Arti3DApp()
{
	if (m_pWindow)
		delete m_pWindow;
	if (m_pDevice)
		delete m_pDevice;
	if (m_pMesh)
		delete m_pMesh;
	if (m_pVertexShader)
		delete m_pVertexShader;
	if (m_pPixelShader)
		delete m_pPixelShader;
	if (m_pBackbuffer)
		delete m_pBackbuffer;
	if (m_pZBuffer)
		delete m_pZBuffer;
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
}

Arti3DResult Arti3DApp::Init()
{
	// Turn on all float point exceptions except inexact exceptions.
	_MM_SET_EXCEPTION_MASK(0);
	_MM_SET_EXCEPTION_MASK(_MM_MASK_INEXACT);

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf_s(stderr, "SDL_Init Failed!\n");
		return ARTI3D_UNKOWN;
	}

	Arti3DResult a3dr = CreateArti3DWindow(&m_pWindow,
		"Arti3DApp", 
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

	//m_pDevice->SetMatrix(ARTI3D_MATRIX_MODEL, a3d::rotate(rotAngle, a3d::vec3(0.0f, 1.0f, 0.0f)));
	m_pDevice->SetMatrix(ARTI3D_MATRIX_MODEL, a3d::mat4(1.0f));
	a3d::mat4 rotMat = a3d::rotate(rotAngle, a3d::vec3(0.0f, 1.0f, 0.0f));
	a3d::vec3 viewDir = a3d::mat3(rotMat) * a3d::vec3(0.0f, 0.0f, 5.0f);
	a3d::mat4 viewMat = a3d::lookAt(a3d::vec3(0.0f, 0.0f, 0.0f), viewDir, a3d::vec3(0.0f, 1.0f, 0.0f));
	m_pDevice->SetMatrix(ARTI3D_MATRIX_VIEW, viewMat);
	m_pDevice->SetMatrix(ARTI3D_MATRIX_PROJECTION, a3d::perspective(45.0f, (float)m_pWindow->m_iWidth / m_pWindow->m_iHeight, 0.1f, 1000.0f));
	m_pDevice->SetViewport(0, 0, m_pWindow->m_iWidth, m_pWindow->m_iHeight);

	tLast = curTime;

	m_pDevice->Begin();
	m_pDevice->ClearColorBuffer(a3d::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	m_pDevice->ClearDepthBuffer();
	
	m_pDevice->SetVertexShader(m_pVertexShader);
	m_pDevice->SetPixelShader(m_pPixelShader);

	m_pMesh->Render();
	
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

	m_pDevice->CreateSurfaceFromWindow(&m_pBackbuffer, m_pWindow);
	m_pDevice->CreateRGBSurface(&m_pZBuffer, m_pWindow->m_iWidth, m_pWindow->m_iHeight, 32, 0, 0, 0, 0);

	pRenderTarget->SetBackBuffer(m_pBackbuffer);
	pRenderTarget->SetZBuffer(m_pZBuffer);

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
	m_pMesh = new Arti3DMesh(m_pDevice);
	m_pMesh->CreateColorCube();

	m_pVertexShader = new SimpleCubeVS();
	m_pPixelShader = new SimpleCubePS();

	m_pDevice->DistributeThreadWorkload();
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

void Arti3DApp::SetupScene2()
{
	m_pMesh = new Arti3DMesh(m_pDevice);
	m_pMesh->CreateTextureCube();

	m_pVertexShader = new CheckboardCubeVS();
	m_pPixelShader = new CheckboardCubePS();
	
	m_pDevice->DistributeThreadWorkload();
		
}

void Arti3DApp::SetupScene3()
{
	m_pMesh = new Arti3DMesh(m_pDevice);
	m_pMesh->CreatePhongCube();

	PhongVS *pVS = new PhongVS();
	PhongPS *pPS = new PhongPS();

	m_pVertexShader = pVS;
	m_pPixelShader = pPS;

	Arti3DMaterial& material = m_pDevice->mRC.globals.material;
	material.vAmbient = a3d::vec3(0.2f, 0.2f, 0.2f);
	material.vDiffuse = a3d::vec3(0.9f, 0.2f, 0.5f);
	material.vSpecular = a3d::vec3(0.5f, 0.5f, 0.5f);
	material.fShinness = 23.0f;

	Arti3DLight& light = m_pDevice->mRC.globals.lights[0];
	light.vPosition = a3d::vec3(10.0f, 10.0f, 10.0f);
	light.vIntensity = a3d::vec3(0.5f, 0.5f, 0.5f);

	m_pDevice->DistributeThreadWorkload();
}

void Arti3DApp::SetupScene4()
{
	m_pMesh = new Arti3DMesh(m_pDevice);
	m_pMesh->CreateSkybox();

	SkyboxVS *pVS = new SkyboxVS();
	SkyboxPS *pPS = new SkyboxPS();

	m_pVertexShader = pVS;
	m_pPixelShader = pPS;

	m_pDevice->DistributeThreadWorkload();
}
