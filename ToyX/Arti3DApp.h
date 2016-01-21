#ifndef _ARTI3DAPP_H_
#define _ARTI3DAPP_H_

#include <string>

#include "Arti3D_Types.h"
#include "Arti3D_ForwardDecl.h"

class Arti3DApp
{
public:
	Arti3DApp();
	~Arti3DApp();

public:
	void Run();

	Arti3DResult Init();

	void RenderScene();

	// Setup Colored Cube Scene.
	void SetupScene();
	
	// Setup Textured Cube Scene.
	void SetupScene2();

	// Setup Scene For Phong Cube.
	void SetupScene3();

	// Setup scene for sky box.
	void SetupScene4();

	void CalculateFPS();

	static void HandleEvent(const SDL_Event& event, Arti3DApp *pApp);
	static void HandleKeyEvent(const SDL_Event& event, Arti3DApp* pApp);

private:

	Arti3DResult CreateArti3DWindow(Arti3DWindow **o_pWindow, const char *pTitle, int x, int y, int width, int height, int flag);
	Arti3DResult CreateAndInitializeDevice(Arti3DDevice **o_pDevice, Arti3DDeviceParameter *pA3DDeviceParameters);


private:
	Arti3DDevice		*m_pDevice;
	Arti3DWindow		*m_pWindow;

	Arti3DMesh			*m_pMesh;
	Arti3DVertexShader	*m_pVertexShader;
	Arti3DPixelShader	*m_pPixelShader;

	Arti3DSurface		*m_pBackbuffer;
	Arti3DSurface		*m_pZBuffer;

	bool				m_bRunning;
	std::string			m_strAppName;
	int					m_iWidth;
	int					m_iHeight;
};


#endif