#include "stdafx.h"
#include "Arti3D_Window.h"
#include "Clock.h"
#include "Arti3D_Device.h"

#include <SDL/SDL.h>

Arti3DWindow::Arti3DWindow(Arti3DDevice* pParent) : m_pParent(pParent)
, m_pWindow(nullptr)
, m_bRunning(false)
{

}

Arti3DWindow::~Arti3DWindow()
{
	if (m_pWindow)
		SDL_DestroyWindow(m_pWindow);
}

Arti3DResult Arti3DWindow::Create(const char *pTitle, int x, int y, int width, int height, uint32_t flag)
{
	m_pWindow = SDL_CreateWindow(pTitle, x, y, width, height, flag);
	if (m_pWindow)
		return ARTI3D_OK;
	return ARTI3D_INVALID_PARAMETER;
}

void Arti3DWindow::HandleEvent(const SDL_Event& event,Arti3DWindow *pWnd)
{
	switch (event.type)
	{
	case SDL_QUIT:
		pWnd->m_bRunning = false;
		break;
	case SDL_KEYDOWN:
		HandleKeyEvent(event,pWnd);
		break;
	default:
		break;
	}
}

void Arti3DWindow::HandleKeyEvent(const SDL_Event& event,Arti3DWindow *pWnd)
{
	switch (event.key.keysym.sym)
	{
	case SDLK_ESCAPE:
		pWnd->m_bRunning = false;
		break;
	default:
		break;
	}
}

void Arti3DWindow::Run()
{
	auto past = 0.0;

	auto last = iv::Clock::GetCurrentTimeMS();

	auto nFrame = 0;

	auto rotAngle = 0.0f;
	const auto rotSpeed = a3d::PI / 20.0f;

	while (m_bRunning)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			HandleEvent(event,this);
		double curTime = iv::Clock::GetCurrentTimeMS();

		past += curTime - last;
		auto dt = (curTime - last) * 0.001;
		rotAngle += rotSpeed * dt;
		if (rotAngle > a3d::TWOPI)
			rotAngle -= a3d::TWOPI;

		m_pParent->SetMatrix(TOY_MATRIX_MODEL, a3d::rotate(rotAngle, a3d::vec3(0.0f, 1.0f, 0.0f)));

		last = curTime;

		if (past >= 1000.0)
		{
			std::cout << "FPS:" << nFrame << std::endl;
			nFrame = 0;
			past = 0.0;
		}
		else
			++nFrame;

		m_pParent->Begin();
		m_pParent->ClearColorBuffer(a3d::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		m_pParent->ClearDepthBuffer();
		m_pParent->DrawMesh_MT();
		m_pParent->End();

		SDL_UpdateWindowSurface(m_pWindow);
	}
}
