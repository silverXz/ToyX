#ifndef _ARTI3D_FORWARDDECL_H_
#define _ARTI3D_FORWARDDECL_H_

#include <memory>

class Arti3DWindow;
class Arti3DDevice;
class Arti3DRenderTarget;
class Arti3DSurface;
class Arti3DVertexBuffer;
class Arti3DVertexLayout;
class Arti3DIndexBuffer;
class Arti3DVertexShader;
class Arti3DPixelShader;
class Arti3DThread;
class Arti3DTile;

typedef std::shared_ptr<Arti3DWindow>		SPWindow;
typedef std::shared_ptr<Arti3DRenderTarget> SPRenderTarget;
typedef std::shared_ptr<Arti3DSurface>		SPSurface;
typedef std::shared_ptr<Arti3DVertexBuffer> SPVertexBuffer;
typedef std::shared_ptr<Arti3DVertexLayout> SPVertexLayout;
typedef std::shared_ptr<Arti3DIndexBuffer>	SPIndexBuffer;
typedef std::shared_ptr<Arti3DVertexShader> SPVertexShader;
typedef std::shared_ptr<Arti3DPixelShader>  SPPixelShader;
typedef std::shared_ptr<Arti3DThread>		SPThread;
typedef std::shared_ptr<Arti3DTile>			SPTile;

typedef Arti3DWindow*						PArti3DWindow;
typedef Arti3DRenderTarget*					PArti3DRenderTarget;
typedef Arti3DSurface*						PArti3DSurface;
typedef Arti3DVertexBuffer*					PArti3DVertexBuffer;
typedef Arti3DVertexLayout*					PArti3DVertexLayout;
typedef Arti3DIndexBuffer*					PArti3DIndexBuffer;
typedef Arti3DVertexShader*					PArti3DVertexShader;
typedef Arti3DPixelShader*			 		PArti3DPixelShader;
typedef Arti3DThread*						PArti3DThread;
typedef Arti3DTile*							PArti3DTile;



// Arti3D Structure Declaration!
struct Arti3DVSOutput;
struct Arti3DVSInput;
struct Arti3DVertexCache;
struct Arti3DFragment;
struct Arti3DDeviceParameter;

typedef	Arti3DVSOutput*						PArti3DVSOutput;
typedef Arti3DVSInput*						PArti3DVSInput;


// SDL Declarations!
struct SDL_Window;
struct SDL_Surface;
union SDL_Event;

#endif