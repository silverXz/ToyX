#ifndef _TOY_TYPES_H_
#define _TOY_TYPES_H_

#include "ToyMath.h"
#include <SDL/SDL.h>
#include <cstdint>
#include <vector>
#include "Shader.h"

#define FLOAT_CAST(x) static_cast<float>(x)


const int g_ciVertexArraySize = 1000;
const int g_ciIndexArraySize = 3 * g_ciVertexArraySize;
const int g_ciCacheSize = 32;
const int g_ciMaxVaryingNum = 12;
const int g_ciMaxClipVertex = 9;
const int g_ciMaxThreadNum = 8;
const int g_ciMaxTextureUnit = 16;

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

struct Arti3DTransformedVertex
{
	toy::vec4 p;
	float varyings[g_ciMaxVaryingNum];
};

struct Arti3DVertexCache
{
	uint32_t				tag;
	Arti3DTransformedVertex	*v;

	Arti3DVertexCache() : tag(UINT_MAX), v(nullptr) {}

	inline void Clear()
	{
		tag = UINT_MAX;
		v = nullptr;
	}
};

struct Toy_Plane
{
	float x, y, z, d;
};

struct Toy_VertexBuffer
{
	Toy_Vertex	vBuffer[g_ciVertexArraySize];
	int			size;
};

struct Toy_IndiceBuffer
{
	uint32_t	iBuffer[g_ciIndexArraySize];
	int			size;
};

struct Toy_TransformedVertexBuffer
{
	Arti3DTransformedVertex tvBuffer[g_ciVertexArraySize];
	int size;
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
	ARTI3D_RANGE_EXCEED,
	ARTI3D_VARYING_EXCEED,
	ARTI3D_OUT_OF_MEMORY
};




struct GlobalUniforms
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
	GlobalUniforms	globals;
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
	ARTI3D_VAF_VECTOR4
};

enum Arti3D_TileCoverage {
	TC_PARTIAL = 0,
	TC_ALL
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
	Arti3D_TileCoverage	coverageType;
};

struct Arti3D_Fragment
{
	int x, y;
	int mask;
	int faceID;
	Arti3D_FragmentCoverage	coverType;
};

struct Arti3D_Tile
{
	std::vector<Arti3D_TiledFace>	aTilizedFace;
	std::vector<Arti3D_Fragment>		aFragment;
	
	Arti3D_TiledFace	**face_index_buffer;
	uint32_t			**index_buffer_size;

	volatile uint32_t	iFragment;

	
	int x, y;
	int w, h;
};

enum Toy3DVertexElementType {
	TOY3D_VT_FLOAT32,
	TOY3D_VT_VECTOR2,
	TOY3D_VT_VECTOR3,
	TOY3D_VT_VECTOR4
};

struct Toy3DVertexElement {
	uint32_t				iVaryingSlot;
	Toy3DVertexElementType	eType;
};

struct UyVertexFormat
{

};



// Inline Helper Functions

inline void SAFE_DELETE_ARRAY(void *ptr)
{
	if (ptr) delete[] ptr, ptr = nullptr;
}

inline void SAFE_DELETE(void *ptr)
{
	if (ptr) delete ptr, ptr = nullptr;
}

inline bool FUNC_FAILED(Arti3DResult result)
{
	return result != ARTI3D_OK;
}


#endif