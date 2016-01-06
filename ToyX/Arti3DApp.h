#ifndef _ARTI3D_H_
#define _ARTI3D_H_

#include <string>

#include "Arti3D_Types.h"

class Arti3DDevice;
class Arti3DWindow;
union SDL_Event;

class Arti3DApp
{
public:
	Arti3DApp();
	~Arti3DApp();

public:
	void Run();

	Arti3DResult Init();

	void RenderScene();
	
	void SetupScene();

	void CalculateFPS();

	static void HandleEvent(const SDL_Event& event, Arti3DApp *pApp);
	static void HandleKeyEvent(const SDL_Event& event, Arti3DApp* pApp);

private:
	Arti3DResult CreateArti3DWindow(Arti3DWindow **o_pWindow, const char *pTitle, int x, int y, int width, int height, int flag);
	Arti3DResult CreateAndInitializeDevice(Arti3DDevice **o_pDevice, Arti3DDeviceParameter *pA3DDeviceParameters);


private:
	Arti3DDevice		*m_pDevice;
	Arti3DWindow		*m_pWindow;

	bool				m_bRunning;
	std::string			m_strAppName;
	int					m_iWidth;
	int					m_iHeight;
};


#endif