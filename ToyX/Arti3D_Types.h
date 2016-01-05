#ifndef _TOY_TYPES_H_
#define _TOY_TYPES_H_

#include "Arti3D_Math.h"
#include <SDL/SDL.h>
#include <cstdint>
#include <vector>
#include "Shader.h"

#define FLOAT_CAST(x) static_cast<float>(x)


const int g_ciMaxVertexNumPerPatch = 3000;
const int g_ciMaxClipVertexNumPerPatch = 5 * g_ciMaxVertexNumPerPatch;
const int g_ciCacheSize = 32;
const int g_ciMaxVaryingNum = 12;
const int g_ciMaxClipVertex = 9;
const int g_ciMaxThreadNum = 10;
const int g_ciMaxTextureUnit = 16;
const int g_ciMaxVSRegister = 8;
const int g_ciMaxFaceNumPerTile = 128;

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

struct Arti3DDeviceParameter
{
	uint32_t	iWidth, iHeight;
	bool		bMultiThread;
};



enum Arti3DClipMask{
	CLIP_POS_X = 1,
	CLIP_NEG_X = 2,
	CLIP_POS_Y = 4,
	CLIP_NEG_Y = 8,
	CLIP_POS_Z = 16,
	CLIP_NEG_Z = 32
};

enum Arti3DMatrixType {
	TOY_MATRIX_MODEL = 0,
	TOY_MATRIX_VIEW,
	TOY_MATRIX_PROJECTION,
	TOY_MATRIX_TYPE_NUM
};

enum GeometryDataType {
	GEOMETRY_VERTEX,
	GEOMETRY_INDICE
};


struct Arti3DVSOutput
{
	a3d::vec4 p;
	float varyings[g_ciMaxVaryingNum];
};

struct Arti3DVertexCache
{
	uint32_t				tag;
	Arti3DVSOutput			vs_output;

	Arti3DVertexCache() : tag(UINT_MAX){}

	inline void Clear()
	{
		tag = UINT_MAX;
	}
};

struct Arti3DPlane
{
	float x, y, z, d;
};

struct Arti3DTransformedFace
{
	float v0x, v0y, v0w;
	float v0v[g_ciMaxVaryingNum];

	int fp1[2];
	int fp2[2];
	int fp3[2];

	a3d::vec2 dw;
	a3d::vec2 dv[g_ciMaxVaryingNum];
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

struct RenderContext
{
	Arti3DVertexShader	pfnVS;
	Arti3DPixelShader	pfnPS;
	Arti3DShaderUniform	globals;
};

struct RenderTarget
{
	SDL_Surface *back_buffer;
	SDL_Surface	*z_buffer;
	SDL_Surface* tex[g_ciMaxTextureUnit];

	int		iTex;


	RenderTarget() : back_buffer(nullptr), z_buffer(nullptr)
	{
		for (auto& x : tex)
			x = nullptr;
		iTex = 0;
	}
	~RenderTarget()
	{
	}

	int AddTexture(SDL_Surface *pSF)
	{
		if (pSF != nullptr)
		{
			std::cerr << "RenderTarget : Null Parameter!\n";
			return -1;
		}
		
		if (iTex >= g_ciMaxTextureUnit)
		{
			std::cerr << "RenderTarget : Max Texture Limits Reached!\n";
			return -1;
		}

		tex[iTex] = pSF;

		return iTex++;
	}
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

enum Arti3DFragmentCoverage
{
	ARTI3D_FC_TILE = 0,
	ARTI3D_FC_BLOCK,
	ARTI3D_FC_MASKED,
};

struct Arti3D_TiledFace
{
	uint32_t		id;
	Arti3DTileCoverage	coverageType;
};

struct Arti3DFragment
{
	int x, y;		// Top Left Coordinates Of This Fragment.
	int mask;		// Coverage Mask, Only Valid When "coverType" = ARTI3D_MASKED.
	int faceID;
	int threadID;
	Arti3DFragmentCoverage	coverType;
};

struct Arti3_DTile
{
	std::vector<Arti3D_TiledFace>	aTilizedFace;
	std::vector<Arti3DFragment>		aFragment;
	
	uint32_t			**ppFaceIndexBuffer;
	uint32_t			*pIndexBufferSize;
	Arti3DTileCoverage	**ppTileCoverage;

	volatile uint32_t	iFragment;

	
	int x, y;
	int w, h;
};


typedef a3d::vec4 ShaderRegister;

struct Arti3DVSInput {
	ShaderRegister ShaderInputs[g_ciMaxVSRegister];
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