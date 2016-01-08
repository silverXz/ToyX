#ifndef _TOY_TYPES_H_
#define _TOY_TYPES_H_

#include <cstdint>
#include <vector>

#include "Arti3D_Math.h"
#include "Arti3D_SSE_Math.h"

#define FLOAT_CAST(x) static_cast<float>(x)


const int ARTI3D_MAX_VERTEX_PER_PATCH		= 3000;
const int ARTI3D_MAX_CLIP_VERTEX_PER_PATCH	= 5 * ARTI3D_MAX_VERTEX_PER_PATCH;
const int ARTI3D_MAX_VERTEX_SHADER_REGISTER = 8;
const int ARTI3D_MAX_FACE_PER_TILE			= 128;

const int ARTI3D_MAX_CACHE_SIZE		= 32;
const int ARTI3D_MAX_VARYING		= 12;
const int ARTI3D_MAX_CLIP_VERTEX	= 9;
const int ARTI3D_MAX_THREAD			= 8;
const int ARTI3D_MAX_TEXTURE_UNIT	= 8;


// If you want to change TILE_SIZE, remember to change TILE_SIZE_SHIFT as well.
// TILE_SIZE = 1 << TILE_SIZE_SHIFT.
const int g_ciTileSize = 16;
const int g_ciTileSizeShift = 4;

const int g_ciBlockSize = 8;

inline uint32_t CvrtToUint32(const a3d::vec4 &color)
{
	const uint32_t mask = ~0xFF;
	uint32_t red = (uint32_t)(255.0f * color.r);
	red = (red & mask) ? 255 : red;
	uint32_t green = (uint32_t)(255.0f * color.g);
	green = (green & mask) ? 255 : green;
	uint32_t blue = (uint32_t)(255.0f * color.b);
	blue = (blue & mask) ? 255 : blue;
	uint32_t alpha = (uint32_t)(255.0f * color.a);
	alpha = (alpha & mask) ? 255 : alpha;
	return ((alpha << 24) | (red << 16) | (green << 8) | blue);
}

enum Arti3DMatrixType {
	ARTI3D_MATRIX_MODEL = 0,
	ARTI3D_MATRIX_VIEW,
	ARTI3D_MATRIX_PROJECTION,
	ARTI3D_MATRIX_TYPE_NUM
};

struct Arti3DPlane
{
	float x, y, z, d;
};

enum Arti3DResult {
	ARTI3D_OK = 0,
	ARTI3D_INVALID_PARAMETER,
	ARTI3D_NULL_PARAMETER,
	ARTI3D_INVALID_ENUM,
	ARTI3D_INVALID_FORMAT,
	ARTI3D_INVALID_BUFFER_SIZE,
	ARTI3D_RANGE_EXCEED,
	ARTI3D_VARYING_EXCEED,
	ARTI3D_OUT_OF_MEMORY,
	ARTI3D_UNKOWN,
};

struct Arti3DShaderUniform
{
	a3d::mat4 model;
	a3d::mat4 view;
	a3d::mat4 projection;
	a3d::mat4 mvp;
	a3d::vec4 viewport;
};

enum Arti3DVertexAttributeFormat
{
	ARTI3D_VAF_FLOAT32,
	ARTI3D_VAF_VECTOR2,
	ARTI3D_VAF_VECTOR3,
	ARTI3D_VAF_VECTOR4,
	ARTI3D_VAF_UNDEFINED
};

enum Arti3DTileCoverage {
	ARTI3D_TC_PARTIAL = 0,
	ARTI3D_TC_ALL
};

struct Arti3DPixelFormat {
	uint32_t	BitsPerPixel;
	uint32_t	BytesPerPixel;
	uint32_t	RMask;
	uint32_t	GMask;
	uint32_t	BMask;
	uint32_t	AMask;
	uint8_t		Rshift;
	uint8_t		Gshift;
	uint8_t		Bshift;
	uint8_t		Ashift;
};

enum Arti3DFormat {
	ARTI3D_FORMAT_R32F,
	ARTI3D_FORMAT_RG88,
	ARTI3D_FORMAT_RGB888,
	ARTI3D_FORMAT_RGBA8888,

	ARTI3D_FORMAT_INDEX8,
	ARTI3D_FORMAT_INDEX16,
	ARTI3D_FORMAT_INDEX32,
	ARTI3D_FORMAT_INVLAID
};

// Inline Helper Functions

#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p),(p) = nullptr;} }
#define SAFE_DELETE(p) {if(p) {delete (p), (p) = nullptr;}}

inline bool FUNC_FAILED(Arti3DResult result)
{
	return result != ARTI3D_OK;
}

inline int iRound(float f)
{
	int retval;
	__asm {
		fld f
			fistp retval
	}
	return retval;
	//return static_cast<int>(f + 0.5f);
}

#endif