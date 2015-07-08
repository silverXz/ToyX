#pragma once

#include <stdint.h>
#include "ToyMath.h"

#define FLOAT_CAST(x) static_cast<float>(x)

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

class ToyRender
{
public:
	ToyRender();
	~ToyRender();

	void InitBuffers(int width, int height, uint32_t* pData,bool depthBuffer);

	// Draw Calls

	// Bresenham algorithm
	void Draw2DLines(int x1, int y1, int x2, int y2, const ToyColor &color);

	void SetPixelColor(int x, int y, const ToyColor& color);


	// Render State Related
	void SetMatrix(MatrixType matrixType, const toy::mat4& m);
	void SetViewport(int x, int y, int width, int height);

	void ClearColorBuffer(const ToyColor& color);
	void ClearDepthBuffer(float cDepth = 0.0f);


private:


private:
	int			mWidth;
	int			mHeight;

	uint32_t*	mPixels;
	float*		mDepthBuffer;

	// Render State
	toy::mat4	mMatrices[TOY_MATRIX_TYPE_NUM];
	toy::vec4	mViewport;
};