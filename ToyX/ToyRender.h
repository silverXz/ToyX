#pragma once

#include <stdint.h>
#include <SDL/SDL.h>
#include <vector>
#include "ToyMath.h"
#include "Shader.h"

#define FLOAT_CAST(x) static_cast<float>(x)


const int VERTEX_ARRAY_SIZE = 1000;
const int INDICE_ARRAY_SIZE = 3 * VERTEX_ARRAY_SIZE;
const int CACHE_SIZE = 32;
const int VARYINGS_NUM = 12;

class ToyColor {
public:
	uint32_t mColor;

	explicit ToyColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255)
	{
		mColor = ((a << 24) | (r << 16) | (g << 8) | b);
	}

	explicit ToyColor(float r, float g, float b, float a = 1.0f)
	{
		uint32_t red = (uint32_t)(255.0f * r);
		red = (red > 255) ? 255 : red;
		uint32_t green = (uint32_t)(255.0f * g);
		green = (green > 255) ? 255 : green;
		uint32_t blue = (uint32_t)(255.0f * b);
		blue = (blue > 255) ? 255 : blue;
		uint32_t alpha = (uint32_t)(255.0f * a);
		alpha = (alpha > 255) ? 255 : alpha;
		mColor = ((alpha << 24) | (red << 16) | (green << 8) | blue);
	}

	uint32_t ToUInt32() const
	{
		return mColor;
	}
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
	Toy_TransformedVertex *v[3];
	
	int fp1[2];
	int fp2[2];
	int fp3[2];

	float v0x, v0y;
	float v0w;

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

	RenderTarget() : back_buffer(nullptr), z_buffer(nullptr) {}
	~RenderTarget()
	{
		SDL_FreeSurface(back_buffer);
		SDL_FreeSurface(z_buffer);
	}
};


class ToyRender
{

public:



public:
	ToyRender();
	~ToyRender();


	void SetRenderTarget(SDL_Surface *cb,SDL_Surface *zb);


	void Begin();

	void End();

	// Draw Calls

	// Bresenham algorithm
	void Draw2DLines(int x1, int y1, int x2, int y2, const ToyColor &color);


	void Draw3DLines(const toy::vec4& p1, const toy::vec4 p2, const ToyColor &color);

	void SetPixelColor(int x, int y, const ToyColor& color);


	void SetVertexShader(VertexShader vs);
	void SetFragmentShader(FragmentShader fs);

	// Render State Related
	void SetMatrix(MatrixType matrixType, const toy::mat4& m);
	void SetViewport(int x, int y, int width, int height);

	void ClearColorBuffer(const ToyColor& color);
	void ClearDepthBuffer(float cDepth = 0.0f);

	void Draw3DSolidTriangle(const toy::vec4& p1,const toy::vec4& p2,const toy::vec4& p3,const ToyColor& c);

	void UploadData(GeometryDataType gdt, void *ptr, uint32_t size);

	void LoadCube();

	void DrawMesh();

private:

	void ProcessV();
	void ProcessR();
	void ProcessP();

	void RasterizeTriangle(Toy_TransformedFace* f);

	inline int iRound(float f)
	{
		return static_cast<int>(f + 0.5f);
	}

	void PostProcessV(Toy_TransformedVertex *v);

	bool EdgeFunc(int a, int b, int c, int x, int y);

	void DrawTriangleScreenSpace(float x1, float y1, float x2, float y2, float x3, float y3, const ToyColor& c);

	void DrawHorizontal2DLine(int x1, int x2, int y, const ToyColor& color);
	void DrawVertical2DLine(int y1, int y2, int x, const ToyColor& color);


	void ComputeGradient(float C, float di21, float di31, float dx21, float dy21, float dx31, float dy31, toy::vec2 *g);

private:

	VertexCache					vCache[CACHE_SIZE];
	Toy_VertexBuffer			vBuffer;
	Toy_IndiceBuffer			iBuffer;

	Toy_TransformedVertex		tvBuffer[VERTEX_ARRAY_SIZE];

	std::vector<Toy_TransformedFace>	faceBuffer;

	Toy_VertexBuffer	tvb;

	RenderTarget		mRT;

	// Render State
	RenderContext	mRC;
};