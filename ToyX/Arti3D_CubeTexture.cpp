#include "stdafx.h"
#include "Arti3D_CubeTexture.h"

#include <SDL/SDL.h>
#include <FreeImage/FreeImage.h>



Arti3DCubeTexture::Arti3DCubeTexture(Arti3DDevice *pDevice) : m_pDevice(pDevice)
{
	for (int i = 0; i < 6; ++i)
		m_pSurface[i] = nullptr;
}

Arti3DResult Arti3DCubeTexture::Create(uint32_t bpp, uint32_t i_iWidth, uint32_t i_iHeight, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask)
{
	for (int i = 0; i < 6; ++i)
	{
		m_pSurface[i] = SDL_CreateRGBSurface(0, i_iWidth, i_iHeight, bpp, rmask, gmask, bmask, amask);

		if (!m_pSurface)
			return ARTI3D_INVALID_FORMAT;

		m_pixelFormat.BitsPerPixel = m_pSurface[i]->format->BitsPerPixel;
		m_pixelFormat.BytesPerPixel = m_pSurface[i]->format->BytesPerPixel;

		m_pixelFormat.RMask = m_pSurface[i]->format->Rmask;
		m_pixelFormat.GMask = m_pSurface[i]->format->Gmask;
		m_pixelFormat.BMask = m_pSurface[i]->format->Bmask;
		m_pixelFormat.AMask = m_pSurface[i]->format->Amask;

		// Uncompleted version!!!!!!!
		m_pixelFormat.Rshift = m_pSurface[i]->format->Rshift;
		m_pixelFormat.Gshift = m_pSurface[i]->format->Gshift;
		m_pixelFormat.Bshift = m_pSurface[i]->format->Bshift;
		m_pixelFormat.Ashift = m_pSurface[i]->format->Ashift;
	}

	return ARTI3D_OK;
}

Arti3DResult Arti3DCubeTexture::Create(const char **pFilePaths)
{
	if (!pFilePaths || !*pFilePaths)
		return ARTI3D_INVALID_PARAMETER;

	for (int i = 0; i < 6; ++i)
	{
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(pFilePaths[i]);

		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(pFilePaths[i]);

		if (fif == FIF_UNKNOWN)
			return ARTI3D_INVALID_PARAMETER;

		if (!FreeImage_FIFSupportsReading(fif))
			return ARTI3D_INVALID_PARAMETER;

		FIBITMAP *pBitmap = FreeImage_Load(fif, pFilePaths[i]);
		if (!pBitmap)
		{
			printf("Failed to load image.\n");
			return ARTI3D_UNKOWN;
		}

		int iBpp = FreeImage_GetBPP(pBitmap);

		FIBITMAP *pBitmap32 = iBpp <= 24 ? FreeImage_ConvertTo32Bits(pBitmap) : pBitmap;

		if (!pBitmap)
			return ARTI3D_UNKOWN;

		assert(FreeImage_GetBPP(pBitmap32) == 32);

		int iWidth = FreeImage_GetWidth(pBitmap32);
		int iHeight = FreeImage_GetHeight(pBitmap32);
		int iSrcPitch = FreeImage_GetPitch(pBitmap32);

		int rMask = FreeImage_GetRedMask(pBitmap32);
		int gMask = FreeImage_GetGreenMask(pBitmap32);
		int bMask = FreeImage_GetBlueMask(pBitmap32);

		m_pSurface[i] = SDL_CreateRGBSurface(0, iWidth, iHeight, 32, rMask, gMask, bMask, 0);

		m_pixelFormat.BitsPerPixel = 32;
		m_pixelFormat.BytesPerPixel = 4;

		m_pixelFormat.RMask = rMask;
		m_pixelFormat.GMask = gMask;
		m_pixelFormat.BMask = bMask;
		m_pixelFormat.AMask = 0;

		switch (rMask)
		{
			// RGBX
		case 0xFF000000:
		{
			m_pixelFormat.Rshift = 24;
			m_pixelFormat.Gshift = 16;
			m_pixelFormat.Bshift = 8;
			m_pixelFormat.Ashift = 0;
			break;
		}
		// XRGB
		case 0x00FF0000:
		{
			m_pixelFormat.Rshift = 16;
			m_pixelFormat.Gshift = 8;
			m_pixelFormat.Bshift = 0;
			m_pixelFormat.Ashift = 24;
			break;
		}
		// BGRA
		case 0x0000FF00:
		{
			m_pixelFormat.Rshift = 8;
			m_pixelFormat.Gshift = 16;
			m_pixelFormat.Bshift = 24;
			m_pixelFormat.Ashift = 0;
		}
		// ABGR
		case 0x000000FF:
		{
			m_pixelFormat.Rshift = 0;
			m_pixelFormat.Gshift = 8;
			m_pixelFormat.Bshift = 16;
			m_pixelFormat.Ashift = 24;
			break;
		}
		default:
			break;
		}

		uint8_t *pSrc = FreeImage_GetBits(pBitmap32);
		uint8_t *pDst = reinterpret_cast<uint8_t*>(pGetPixelsDataPtr(i));
		int iDstPitch = iGetPitch();

		for (int i = 0; i < iHeight; ++i)
		{
			memcpy(pDst, pSrc, iSrcPitch);
			pSrc += iSrcPitch;
			pDst += iDstPitch;
		}

		if (iBpp <= 24)
			FreeImage_Unload(pBitmap);
		FreeImage_Unload(pBitmap32);
	}

	return ARTI3D_OK;
}

void* Arti3DCubeTexture::pGetPixelsDataPtr(int eID)
{
	return m_pSurface[eID]->pixels;
}

int Arti3DCubeTexture::iGetBitPerPixel() const
{
	return m_pixelFormat.BitsPerPixel;
}

int Arti3DCubeTexture::iGetPitch() const
{
	return m_pSurface[0]->pitch;
}

int Arti3DCubeTexture::iGetWidth() const
{
	return m_pSurface[0]->w;
}

int Arti3DCubeTexture::iGetHeight() const
{
	return m_pSurface[0]->h;
}
