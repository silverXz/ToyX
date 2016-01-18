#include "stdafx.h"
#include "Arti3D_ShaderBase.h"
#include "Arti3D_ForwardDecl.h"
#include "Arti3D_Device.h"
#include "Arti3D_Surface.h"


SSE_Color3 Arti3DPixelShader::SampleTexture(Arti3DSurface *pSurface, SSE_Float& fU, SSE_Float& fV)
{
	__m128 inv = _mm_set1_ps(1.0f / 255.0f);

	__m128 zero = _mm_set1_ps(0.0f);
	__m128 one	= _mm_set1_ps(1.0f);

	fU = SSE_Clamp(fU, zero, one);
	fV = SSE_Clamp(fV, zero, one);
	
	int w = pSurface->iGetWidth();
	int h = pSurface->iGetHeight();

	__m128 tU = _mm_mul_ps(fU, _mm_set_ps1(static_cast<float>(w - 1)));
	__m128 tV = _mm_mul_ps(fV, _mm_set_ps1(static_cast<float>(h - 1)));

	__m128i iU = _mm_cvtps_epi32(tU);
	__m128i iV = _mm_cvtps_epi32(tV);

	int ipitch = pSurface->iGetPitch();
	int iBytesPerPixel = pSurface->iGetBitPerPixel() / 8;

	__m128i vOffset = SSE_Multiply_Trunc(iV, _mm_set1_epi32(ipitch));
	__m128i uOffset = SSE_Multiply_Trunc(iU, _mm_set1_epi32(iBytesPerPixel));

	__m128i iSamples = _mm_add_epi32(vOffset, uOffset);

	uint8_t *ps = reinterpret_cast<uint8_t*>(pSurface->pGetPixelsDataPtr());

	uint32_t color0 = *reinterpret_cast<uint32_t*>(&ps[_mm_extract_epi32(iSamples, 0)]);
	uint32_t color1 = *reinterpret_cast<uint32_t*>(&ps[_mm_extract_epi32(iSamples, 1)]);
	uint32_t color2 = *reinterpret_cast<uint32_t*>(&ps[_mm_extract_epi32(iSamples, 2)]);
	uint32_t color3 = *reinterpret_cast<uint32_t*>(&ps[_mm_extract_epi32(iSamples, 3)]);

	__m128i color = _mm_set_epi32(color3, color2, color1, color0);
	
	SSE_Color3 retVal;
	
	const Arti3DPixelFormat& format = pSurface->m_pixelFormat;
	
	retVal.r = _mm_cvtepi32_ps(_mm_srl_epi32(_mm_and_si128(color, _mm_set1_epi32(format.RMask)), _mm_set_epi32(0, 0, 0, format.Rshift)));
	retVal.g = _mm_cvtepi32_ps(_mm_srl_epi32(_mm_and_si128(color, _mm_set1_epi32(format.GMask)), _mm_set_epi32(0, 0, 0, format.Gshift)));
	retVal.b = _mm_cvtepi32_ps(_mm_srl_epi32(_mm_and_si128(color, _mm_set1_epi32(format.BMask)), _mm_set_epi32(0, 0, 0, format.Bshift)));
	
	retVal.r = _mm_mul_ps(retVal.r, inv);
	retVal.g = _mm_mul_ps(retVal.g, inv);
	retVal.b = _mm_mul_ps(retVal.b, inv);

	return retVal;
}
