#include "stdafx.h"
#include "Arti3D_Mesh.h"
#include "Arti3D_Device.h"
#include "Arti3D_VertexBuffer.h"
#include "Arti3D_IndexBuffer.h"
#include "Arti3D_Surface.h"
#include "Arti3D_Math.h"

Arti3DMesh::Arti3DMesh(Arti3DDevice *pDevice) : m_pDevice(pDevice)
, m_pVertexBuffer(nullptr)
, m_pIndexBuffer(nullptr)
{

}

Arti3DMesh::~Arti3DMesh()
{
	if (m_pVertexBuffer)
		delete m_pVertexBuffer;
	if (m_pIndexBuffer)
		delete m_pIndexBuffer;
}

void Arti3DMesh::Render()
{
	m_pDevice->BindVertexBuffer(m_pVertexBuffer);
	m_pDevice->BindIndexBuffer(m_pIndexBuffer);
	m_pDevice->DrawMesh_MT();
}

void Arti3DMesh::CreateColorCube()
{
	if (!m_pDevice)
		return;

	// Create Cube Data.
	const float len = 2.0f;
	std::vector<std::vector<float>> xv{
		{ -len, len, len, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },	//0
		{ len, len, len, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },	//1
		{ len, len, -len, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f },	//2
		{ -len, len, -len, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f },	//3
		{ -len, -len, len, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f },	//4
		{ len, -len, len, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },	//5
		{ len, -len, -len, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },	//6
		{ -len, -len, -len, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }	//7
	};
	const uint32_t iVertex = 8;

	// Create Vertex Buffer.
	Arti3DVertexLayout *pVertexLayout = nullptr;
	Arti3DVertexAttributeFormat vaf[] = { ARTI3D_VAF_VECTOR4, ARTI3D_VAF_VECTOR4 };
	m_pDevice->CreateVertexLayout(&pVertexLayout, 2, vaf);
	m_pDevice->CreateVertexBuffer(&m_pVertexBuffer, pVertexLayout, iVertex);

	// Upload Cube Data To VertexBuffer
	int iStride = m_pVertexBuffer->iGetStride();
	for (int i = 0; i < iVertex; ++i)
	{
		void *pDest = nullptr;
		m_pVertexBuffer->GetPointer(i * iStride, &pDest);
		memcpy(pDest, &xv[i][0], iStride);
	}

	m_pDevice->BindVertexBuffer(m_pVertexBuffer);

	// Create And Upload Data To Index Buffer.
	m_pDevice->CreateIndexBuffer(&m_pIndexBuffer, 36 * sizeof(uint32_t), ARTI3D_FORMAT_INDEX32);
	uint32_t xid[] = { 0, 1, 2, 0, 2, 3, 0, 4, 5, 0, 5, 1, 1, 5, 6, 1, 6, 2, 4, 7, 6, 4, 6, 5, 0, 3, 7, 0, 7, 4, 3, 2, 6, 3, 6, 7 };
	void *pDest = nullptr;
	m_pIndexBuffer->GetPointer(0, &pDest);
	memcpy(pDest, xid, sizeof(xid));
	m_pDevice->BindIndexBuffer(m_pIndexBuffer);
}

void Arti3DMesh::CreateTextureCube()
{
	if (!m_pDevice)
		return;

	// Create Cube Data.
	const float len = 2.0f;

	std::vector<std::vector<float>> xv{
		{ -len, len, len, 1.0f, 0.0f, 1.0f },	//0,0 - front
		{ -len, -len, len, 1.0f, 0.0f, 0.0f },	//1,4 - front
		{ len, -len, len, 1.0f, 1.0f, 0.0f },	//2,5 - front
		{ len, len, len, 1.0f, 1.0f, 1.0f },    //3,1 - front

		{ len, len, len, 1.0f, 0.0f, 1.0f },	//4,1 - right
		{ len, -len, len, 1.0f, 0.0f, 0.0f },	//5,5 - right
		{ len, -len, -len, 1.0f, 1.0f, 0.0f },	//6,6 - right
		{ len, len, -len, 1.0f, 1.0f, 1.0f },	//7,2 - right

		{ len, len, -len, 1.0f, 0.0f, 1.0f },	//8,2 - back
		{ len, -len, -len, 1.0f, 0.0f, 0.0f },	//9,6 - back
		{ -len, -len, -len, 1.0f, 1.0f, 0.0f },	//10,7 - back
		{ -len, len, -len, 1.0f, 1.0f, 1.0f },	//11,3 - back

		{ -len, len, -len, 1.0f, 0.0f, 1.0f },	//12,3 - right
		{ -len, -len, -len, 1.0f, 0.0f, 0.0f },	//13,7 - right
		{ -len, -len, len, 1.0f, 1.0f, 0.0f },	//14,4 - right
		{ -len, len, len, 1.0f, 1.0f, 1.0f },	//15,0 - right

		{ -len, len, -len, 1.0f, 0.0f, 1.0f },	//16,3 - up
		{ -len, len, len, 1.0f, 0.0f, 0.0f },	//17,0 - up
		{ len, len, len, 1.0f, 1.0f, 0.0f },	//18,1 - up
		{ len, len, -len, 1.0f, 1.0f, 1.0f },	//19,2 - up

		{ -len, -len, len, 1.0f, 0.0f, 1.0f },	//20,4 - down
		{ -len, -len, -len, 1.0f, 0.0f, 0.0f },	//21,7 - down
		{ len, -len, -len, 1.0f, 1.0f, 0.0f },	//22,6 - down
		{ len, -len, len, 1.0f, 1.0f, 1.0f }		//23,5 - down
	};

	int iVertex = xv.size();

	Arti3DVertexLayout *pVertexLayout = nullptr;
	Arti3DVertexAttributeFormat vaf[] = { ARTI3D_VAF_VECTOR4, ARTI3D_VAF_VECTOR2 };
	m_pDevice->CreateVertexLayout(&pVertexLayout, 2, vaf);
	m_pDevice->CreateVertexBuffer(&m_pVertexBuffer, pVertexLayout, iVertex);

	int iStride = m_pVertexBuffer->iGetStride();

	for (int i = 0; i < iVertex; ++i)
	{
		void *pDest = nullptr;
		m_pVertexBuffer->GetPointer(i * iStride, &pDest);
		memcpy(pDest, &xv[i][0], iStride);
	}

	m_pDevice->BindVertexBuffer(m_pVertexBuffer);

	// Create and upload index buffer data.
	m_pDevice->CreateIndexBuffer(&m_pIndexBuffer, 36 * sizeof(uint32_t), ARTI3D_FORMAT_INDEX32);
	uint32_t xid[] = { 0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23 };

	void *pDest = nullptr;
	m_pIndexBuffer->GetPointer(0, &pDest);
	memcpy(pDest, xid, sizeof(xid));

	m_pDevice->BindIndexBuffer(m_pIndexBuffer);

	// Create and upload texture data.
	int iTexWidth = 320;
	int iTexHeight = 320;

	PArti3DSurface pSurface = nullptr;

	Arti3DResult a3dr = m_pDevice->CreateRGBSurface(&pSurface, iTexWidth, iTexHeight, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

	if (a3dr != ARTI3D_OK)
		return;

	int iBoardLength = 32;

	uint32_t white = 0x00FF0000;
	uint32_t black = 0xFF000000;

	uint32_t cc = white;

	uint32_t *pp = (uint32_t*)pSurface->pGetPixelsDataPtr();

	for (int i = 0; i < iTexHeight; ++i)
	{
		int br = (i & iBoardLength);
		for (int j = 0; j < iTexWidth; ++j)
		{
			cc = (j & iBoardLength) ^ br ? black : white;
			pp[i * iTexWidth + j] = cc;
		}
	}
	m_pDevice->AttachTextureUnit(pSurface, 0);
}

void Arti3DMesh::LoadFromFile(const char *pFilePath)
{

}

