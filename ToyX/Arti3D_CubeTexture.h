#ifndef _H_ARTI3D_CUBE_TEXTURE_H_ 
#define _H_ARTI3D_CUBE_TEXTURE_H_

#include "Arti3D_Types.h"
#include "Arti3D_ForwardDecl.h"

enum Arti3DCubeMapID {
	ARTI3D_CUBE_TEXTURE_POS_X = 0,
	ARTI3D_CUBE_TEXTURE_NEG_X,
	ARTI3D_CUBE_TEXTURE_POS_Y,
	ARTI3D_CUBE_TEXTURE_NEG_Y,
	ARTI3D_CUBE_TEXTURE_POS_Z,
	ARTI3D_CUBE_TEXTURE_NEG_Z,
	ARTI3D_CUBE_TEXTURE_COUNT
};


class Arti3DCubeTexture {
protected:
	friend class Arti3DDevice;
	friend class Arti3DPixelShader;

	Arti3DCubeTexture(Arti3DDevice *pDevice);

	// Instance Could Only Be Create By Arti3DDevice
	// @param bpp : bits per pixel
	// @param i_iWidth : Width Of The Surface.
	// @param i_iHeight : Height Of The Surface.
	// @param rmask : red channel mask.
	// @param gmask : greed channel mask.
	// @param bmask : blue channel mask.
	// @param amask : alpha channel mask.
	// @return ARTI3D_OK If This Function Succeeds.
	Arti3DResult Create(uint32_t bpp, uint32_t i_iWidth, uint32_t i_iHeight, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask);

	// Create surface from file.
	// @param pFilepath : File path.
	Arti3DResult Create(const char **pFilePaths);

public:
	~Arti3DCubeTexture();

	void*	pGetPixelsDataPtr(int eID);

	int iGetBitPerPixel() const;
	int iGetPitch() const;
	int iGetWidth() const;
	int iGetHeight() const;

	Arti3DFormat	fmtGetFormat() const;

private:
	Arti3DDevice		*m_pDevice;

	SDL_Surface			*m_pSurface[ARTI3D_CUBE_TEXTURE_COUNT];

	Arti3DPixelFormat	m_pixelFormat;
};


#endif