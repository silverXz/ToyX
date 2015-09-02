#ifndef _TOY_TYPES_H_
#define _TOY_TYPES_H_

#include "ToyMath.h"
#include <SDL/SDL.h>
#include <cstdint>

#define FLOAT_CAST(x) static_cast<float>(x)


const int VERTEX_ARRAY_SIZE = 1000;
const int INDICE_ARRAY_SIZE = 3 * VERTEX_ARRAY_SIZE;
const int CACHE_SIZE = 32;
const int VARYINGS_NUM = 12;
const int CLIP_VERTEX_MAX = 9;

const int TEXTURE_UNIT_MAX = 16;

// If you want to change TILE_SIZE, remember to change TILE_SIZE_SHIFT as well.
// TILE_SIZE = 1 << TILE_SIZE_SHIFT.
const int TILE_SIZE = 16;
const int TILE_SIZE_SHIFT = 4;

const int BLOCK_SIZE = 8;

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

enum ClipMask{
	CLIP_POS_X = 1,
	CLIP_NEG_X = 2,
	CLIP_POS_Y = 4,
	CLIP_NEG_Y = 8,
	CLIP_POS_Z = 16,
	CLIP_NEG_Z = 32
};

enum MatrixType {
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

struct Toy_TransformedVertex
{
	toy::vec4 p;
	float varyings[VARYINGS_NUM];
};

struct VertexCache
{
	uint32_t				tag;
	Toy_TransformedVertex	*v;

	VertexCache() : tag(UINT_MAX), v(nullptr) {}

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
	Toy_Vertex	vBuffer[VERTEX_ARRAY_SIZE];
	int			size;
};

struct Toy_IndiceBuffer
{
	uint32_t	iBuffer[INDICE_ARRAY_SIZE];
	int			size;
};

struct Toy_TransformedVertexBuffer
{
	Toy_TransformedVertex tvBuffer[VERTEX_ARRAY_SIZE];
	int size;
};

struct Toy_TransformedFace
{
	float v0x, v0y, v0w;
	float v0v[VARYINGS_NUM];

	int fp1[2];
	int fp2[2];
	int fp3[2];

	toy::vec2 dw;
	toy::vec2 dv[VARYINGS_NUM];
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
	SDL_Surface* tex[TEXTURE_UNIT_MAX];

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
		
		if (iTex >= TEXTURE_UNIT_MAX)
		{
			std::cerr << "RenderTarget : Max Texture Limits Reached!\n";
			return -1;
		}

		tex[iTex] = pSF;

		return iTex++;
	}
};

enum TileCoverage {
	TC_PARTIAL = 0,
	TC_ALL
};

enum FragmentCoverage
{
	FC_TILE = 0,
	FC_BLOCK,
	FC_FRAGMENT,
};

struct Toy_TilizedFace
{
	uint32_t		id;
	TileCoverage	coverageType;
};

struct Toy_Fragment
{
	int x, y;
	int mask;
	int faceID;
	FragmentCoverage	coverType;
};

struct Toy_Tile
{
	std::vector<Toy_TilizedFace>	aTilizedFace;
	std::vector<Toy_Fragment>		aFragment;
	int x, y;
	int w, h;
};



#endif