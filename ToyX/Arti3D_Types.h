#ifndef _TOY_TYPES_H_
#define _TOY_TYPES_H_

#include "ToyMath.h"
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
const int g_ciMaxThreadNum = 8;
const int g_ciMaxTextureUnit = 16;
const int g_ciMaxVSRegister = 8;
const int g_ciMaxFaceNumPerTile = 128;

// If you want to change TILE_SIZE, remember to change TILE_SIZE_SHIFT as well.
// TILE_SIZE = 1 << TILE_SIZE_SHIFT.
const int g_ciTileSize = 16;
const int g_ciTileSizeShift = 4;

const int g_ciBlockSize = 8;

class ToyColor {
public:
	uint32_t mColor;

	explicit ToyColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255)
	{
		mColor = ((a << 24) | (r << 16) | (g << 8) | b);
	}

	inline explicit ToyColor(float r, float g, float b, float a = 1.0f)
	{
		const uint32_t mask = ~0xFF;
		uint32_t red = (uint32_t)(255.0f * r);
		red = (red & mask) ? 255 : red;
		uint32_t green = (uint32_t)(255.0f * g);
		green = (green & mask) ? 255 : green;
		uint32_t blue = (uint32_t)(255.0f * b);
		blue = (blue & mask) ? 255 : blue;
		uint32_t alpha = (uint32_t)(255.0f * a);
		alpha = (alpha & mask) ? 255 : alpha;
		mColor = ((alpha << 24) | (red << 16) | (green << 8) | blue);
	}

	uint32_t ToUInt32() const
	{
		return mColor;
	}
};

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


struct Toy_Vertex
{
	toy::vec4 p;
	toy::vec4 c;
	toy::vec4 n;
	float u, v;
};

struct Arti3DVSOutput
{
	toy::vec4 p;
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

struct Toy_Plane
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

	toy::vec2 dw;
	toy::vec2 dv[g_ciMaxVaryingNum];
};


enum Arti3DResult {
	ARTI3D_OK = 0,
	ARTI3D_INVALID_PARAMETER,
	ARTI3D_NULL_PARAMETER,
	ARTI3D_INVALID_ENUM,
	ARTI3D_INVALID_FORMAT,
	ARTI3D_RANGE_EXCEED,
	ARTI3D_VARYING_EXCEED,
	ARTI3D_OUT_OF_MEMORY
};




struct Arti3DShaderUniform
{
	toy::mat4 model;
	toy::mat4 view;
	toy::mat4 projection;
	toy::mat4 mvp;
	toy::vec4 viewport;
};

struct RenderContext
{
	VertexShader	vs;
	FragmentShader	fs;
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
	ARTI3D_R32F,
	ARTI3D_R32G32F,
	ARTI3D_R32G32B32F,
	ARTI3D_R32G32B32A32F,

	ARTI3D_INDEX16,
	ARTI3D_INDEX32
};

enum Arti3D_FragmentCoverage
{
	FC_TILE = 0,
	FC_BLOCK,
	FC_FRAGMENT,
};

struct Arti3D_TiledFace
{
	uint32_t		id;
	Arti3DTileCoverage	coverageType;
};

struct Arti3DFragment
{
	int x, y;
	int mask;
	int faceID;
	Arti3D_FragmentCoverage	coverType;
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


typedef toy::vec4 ShaderRegister;

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