#include "stdafx.h"
#include "Arti3D_ShaderBase.h"
#include "Arti3D_ForwardDecl.h"
#include "Arti3D_Device.h"
#include "Arti3D_Surface.h"
#include "Arti3D_CubeTexture.h"


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

SSE_Color3 Arti3DPixelShader::SampleCubeTexture(Arti3DCubeTexture *pCubeTexture, SSE_Float &fU, SSE_Float &fV, SSE_Float &fW)
{
	__m128 fZero = _mm_set1_ps(0.0f);
	__m128 fOne = _mm_set1_ps(1.0f);
	__m128 fCU = fZero, fCV = fZero, fInvMag = fZero;
	__m128i	iZero = _mm_set1_epi32(0);
	__m128i	iTexID = iZero;
	__m128i iLeft;

	__m128 fAbsU = SSE_Abs(fU);
	__m128 fAbsV = SSE_Abs(fV);
	__m128 fAbsW = SSE_Abs(fW);

	__m128 fInvAbsU = _mm_div_ps(fOne, fAbsU);
	__m128 fInvAbsV = _mm_div_ps(fOne, fAbsV);
	__m128 fInvAbsW = _mm_div_ps(fOne, fAbsW);

	// If U dominate.
	__m128i iUgeV = *(__m128i*)&_mm_cmpge_ps(fAbsU, fAbsV);
	__m128i iUgeW = *(__m128i*)&_mm_cmpge_ps(fAbsU, fAbsW);
	__m128i iUVW = _mm_and_si128(iUgeV, iUgeW);

	// Bitwise Not
	iLeft = _mm_xor_si128(iUVW, _mm_set1_epi32(0xFFFFFFFF));

	// Sample Positive X.
	__m128i iUge0 = *(__m128i*)&_mm_cmpge_ps(fU, fZero);
	__m128i iReady_posx = _mm_and_si128(iUVW, iUge0);
	fCU = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posx, *(__m128i*)&SSE_Negative(fW)), *(__m128i*)&fCU);
	fCV = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posx, *(__m128i*)&SSE_Negative(fV)), *(__m128i*)&fCV);
	iTexID = _mm_or_si128(_mm_and_si128(iReady_posx, _mm_set1_epi32(0)), iTexID);
	fInvMag = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posx, *(__m128i*)&fInvAbsU), *(__m128i*)&fInvMag);

	// Sample Negative X.
	__m128i iUlt0 = *(__m128i*)&_mm_cmplt_ps(fU, fZero);
	__m128i iReady_negx = _mm_and_si128(iUVW, iUlt0);
	fCU = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negx, *(__m128i*)&fW), *(__m128i*)&fCU);
	fCV = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negx, *(__m128i*)&SSE_Negative(fV)), *(__m128i*)&fCV);
	iTexID = _mm_or_si128(_mm_and_si128(iReady_negx, _mm_set1_epi32(1)), iTexID);
	fInvMag = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negx, *(__m128i*)&fInvAbsU), *(__m128i*)&fInvMag);

// 	int left2calc = _mm_movemask_ps(*(__m128*)&iLeft);
// 	if (left2calc != 0) // we don't have to consider other conditioins.
// 	{
		// If V dominate.
		__m128i iVgeU = *(__m128i*)&_mm_cmpge_ps(fAbsV, fAbsU);
		__m128i iVgeW = *(__m128i*)&_mm_cmpge_ps(fAbsV, fAbsW);
		__m128i iVUW = _mm_and_si128(_mm_and_si128(iVgeU, iVgeW), iLeft);

		// Sample Positive Y.
		__m128i iVge0 = *(__m128i*)&_mm_cmpge_ps(fV, fZero);
		__m128i iReady_posy = _mm_and_si128(iVUW, iVge0);
		fCU = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posy, *(__m128i*)&fU), *(__m128i*)&fCU);
		fCV = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posy, *(__m128i*)&fW), *(__m128i*)&fCV);
		iTexID = _mm_or_si128(_mm_and_si128(iReady_posy, _mm_set1_epi32(2)), iTexID);
		fInvMag = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posy, *(__m128i*)&fInvAbsV), *(__m128i*)&fInvMag);

		// Sample Nagative Y.
		__m128i iVlt0 = *(__m128i*)&_mm_cmplt_ps(fV, fZero);
		__m128i iReady_negy = _mm_and_si128(iVUW, iVlt0);
		fCU = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negy, *(__m128i*)&fU), *(__m128i*)&fCU);
		fCV = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negy, *(__m128i*)&SSE_Negative(fW)), *(__m128i*)&fCV);
		iTexID = _mm_or_si128(_mm_and_si128(iReady_negy, _mm_set1_epi32(3)), iTexID);
		fInvMag = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negy, *(__m128i*)&fInvAbsV), *(__m128i*)&fInvMag);
		
		iLeft = _mm_xor_si128(_mm_or_si128(iVUW, iUVW), _mm_set1_epi32(0xFFFFFFFF));
//		left2calc = _mm_movemask_ps(*(__m128*)&iLeft);
// 		if (left2calc != 0)
// 		{
			// If W dominate.
			__m128i iWgeU = *(__m128i*)&_mm_cmpge_ps(fAbsW, fAbsU);
			__m128i iWgeV = *(__m128i*)&_mm_cmpge_ps(fAbsW, fAbsV);
			__m128i iWUV = _mm_and_si128(_mm_and_si128(iWgeU, iWgeV),iLeft);

			// Sample Positive Z.
			__m128i iWge0 = *(__m128i*)&_mm_cmpge_ps(fW, fZero);
			__m128i iReady_posz = _mm_and_si128(iWUV, iWge0);
			fCU = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posz, *(__m128i*)&fU), *(__m128i*)&fCU);
			fCV = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posz, *(__m128i*)&SSE_Negative(fV)), *(__m128i*)&fCV);
			iTexID = _mm_or_si128(_mm_and_si128(iReady_posz, _mm_set1_epi32(4)), iTexID);
			fInvMag = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_posz, *(__m128i*)&fInvAbsW), *(__m128i*)&fInvMag);

			// Sample Negative Z.
			__m128i iWlt0 = *(__m128i*)&_mm_cmplt_ps(fW, fZero);
			__m128i iReady_negz = _mm_and_si128(iWUV, iWlt0);
			fCU = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negz, *(__m128i*)&SSE_Negative(fU)), *(__m128i*)&fCU);
			fCV = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negz, *(__m128i*)&SSE_Negative(fV)), *(__m128i*)&fCV);
			iTexID = _mm_or_si128(_mm_and_si128(iReady_negz, _mm_set1_epi32(5)), iTexID);
			fInvMag = *(__m128*)&_mm_or_si128(_mm_and_si128(iReady_negz, *(__m128i*)&fInvAbsW), *(__m128i*)&fInvMag);
// 		}
// 	}

	// Translate to valid texture coordinates.
	__m128 fHalf = _mm_set_ps1(0.5f);
	__m128 tU = _mm_add_ps(_mm_mul_ps(fCU, _mm_mul_ps(fInvMag, fHalf)), fHalf);
	__m128 tV = _mm_add_ps(_mm_mul_ps(fCV, _mm_mul_ps(fInvMag, fHalf)), fHalf);

	//.........................................................................................
	// To Be Done.
	int id0 = _mm_extract_epi32(iTexID, 0);
	int id1 = _mm_extract_epi32(iTexID, 1);
	int id2 = _mm_extract_epi32(iTexID, 2);
	int id3 = _mm_extract_epi32(iTexID, 3);

	__m128 xfU = SSE_Clamp(tU, fZero, fOne);
	__m128 xfV = SSE_Clamp(tV, fZero, fOne);

	int w = pCubeTexture->iGetWidth();
	int h = pCubeTexture->iGetHeight();

	tU = _mm_mul_ps(xfU, _mm_set_ps1(static_cast<float>(w - 1)));
	tV = _mm_mul_ps(xfV, _mm_set_ps1(static_cast<float>(h - 1)));

	__m128i iU = _mm_cvtps_epi32(tU);
	__m128i iV = _mm_cvtps_epi32(tV);

	int ipitch = pCubeTexture->iGetPitch();
	int iBytesPerPixel = pCubeTexture->iGetBitPerPixel() / 8;

	__m128i vOffset = SSE_Multiply_Trunc(iV, _mm_set1_epi32(ipitch));
	__m128i uOffset = SSE_Multiply_Trunc(iU, _mm_set1_epi32(iBytesPerPixel));

	__m128i iSamples = _mm_add_epi32(vOffset, uOffset);

	uint8_t *ps0 = reinterpret_cast<uint8_t*>(pCubeTexture->pGetPixelsDataPtr(id0));
	uint8_t *ps1 = reinterpret_cast<uint8_t*>(pCubeTexture->pGetPixelsDataPtr(id1));
	uint8_t *ps2 = reinterpret_cast<uint8_t*>(pCubeTexture->pGetPixelsDataPtr(id2));
	uint8_t *ps3 = reinterpret_cast<uint8_t*>(pCubeTexture->pGetPixelsDataPtr(id3));

	uint32_t color0 = *reinterpret_cast<uint32_t*>(&ps0[_mm_extract_epi32(iSamples, 0)]);
	uint32_t color1 = *reinterpret_cast<uint32_t*>(&ps1[_mm_extract_epi32(iSamples, 1)]);
	uint32_t color2 = *reinterpret_cast<uint32_t*>(&ps2[_mm_extract_epi32(iSamples, 2)]);
	uint32_t color3 = *reinterpret_cast<uint32_t*>(&ps3[_mm_extract_epi32(iSamples, 3)]);

	__m128i color = _mm_set_epi32(color3, color2, color1, color0);

	SSE_Color3 retVal;

	const Arti3DPixelFormat& format = pCubeTexture->m_pixelFormat;

	retVal.r = _mm_cvtepi32_ps(_mm_srl_epi32(_mm_and_si128(color, _mm_set1_epi32(format.RMask)), _mm_set_epi32(0, 0, 0, format.Rshift)));
	retVal.g = _mm_cvtepi32_ps(_mm_srl_epi32(_mm_and_si128(color, _mm_set1_epi32(format.GMask)), _mm_set_epi32(0, 0, 0, format.Gshift)));
	retVal.b = _mm_cvtepi32_ps(_mm_srl_epi32(_mm_and_si128(color, _mm_set1_epi32(format.BMask)), _mm_set_epi32(0, 0, 0, format.Bshift)));

	__m128 inv = _mm_set1_ps(1.0f / 255.0f);

	retVal.r = _mm_mul_ps(retVal.r, inv);
	retVal.g = _mm_mul_ps(retVal.g, inv);
	retVal.b = _mm_mul_ps(retVal.b, inv);

	return retVal;
}
