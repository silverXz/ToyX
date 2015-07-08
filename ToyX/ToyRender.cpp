#include "stdafx.h"
#include "ToyRender.h"
#include <assert.h>

using namespace toy;

ToyRender::ToyRender() : mWidth(800), mHeight(600), mPixels(0), mDepthBuffer(0)
{

}

ToyRender::~ToyRender()
{
	if (mDepthBuffer)
		delete[] mDepthBuffer;
}

void ToyRender::InitBuffers(int width, int height, uint32_t* pData, bool depthBuffer)
{
	mWidth = width;
	mHeight = height;
	mPixels = pData;

	if (depthBuffer)
		mDepthBuffer = new float[width * height];
}

void ToyRender::ClearColorBuffer(const ToyColor& color)
{
	for (int i = 0; i < mWidth * mHeight; ++i)
		mPixels[i] = color.ToUInt32();
}

void ToyRender::ClearDepthBuffer(float cDepth)
{
	if (!mDepthBuffer)
		return;
	for (int i = 0; i < mWidth * mHeight; ++i)
		mDepthBuffer[i] = cDepth;
}

void ToyRender::SetMatrix(MatrixType matrixType, const toy::mat4& m)
{
	mMatrices[matrixType] = m;
}

void ToyRender::SetViewport(int x, int y, int width, int height)
{
	mViewport = vec4(FLOAT_CAST(x),FLOAT_CAST(y), FLOAT_CAST(width), FLOAT_CAST(height));
}

void ToyRender::Draw2DLines(int x1, int y1, int x2, int y2, const ToyColor &color)
{
}

void ToyRender::SetPixelColor(int x, int y, const ToyColor& color)
{
	assert(x >= 0 && y >= 0);
	if (x >= mWidth || y >= mHeight)
		return;
	mPixels[y * mWidth + x] = color.ToUInt32();
}
