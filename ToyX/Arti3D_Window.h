#ifndef _ARTI3D_WINDOW_H_
#define _ARTI3D_WINDOW_H_

#include "Arti3D_Types.h"

struct SDL_Window;
union SDL_Event;

class Arti3DWindow
{
protected:
	friend class Arti3DDevice;
	friend class Arti3DSurface;

	Arti3DWindow(Arti3DDevice* pParent);
	~Arti3DWindow();


	Arti3DResult Create(const char *pTitle,int x,int y,int width,int height,uint32_t flag);


public:
	static void HandleEvent(const SDL_Event& event,Arti3DWindow *pWnd);
	static void HandleKeyEvent(const SDL_Event& event,Arti3DWindow* pWnd);

	void Run();

private:
	Arti3DDevice	*m_pParent;

	SDL_Window		*m_pWindow;

	bool			m_bRunning;
};


#endif