#ifndef _ARTI3D_MESH_H_
#define _ARTI3D_MESH_H_

#include "Arti3D_ForwardDecl.h"
#include "Arti3D_Types.h"

class Arti3DMesh
{
public:
	Arti3DMesh(Arti3DDevice *pDevice);
	~Arti3DMesh();

	void Render();

	void CreateColorCube();
	void CreateTextureCube();
	
	void LoadFromFile(const char *pFilePath);

private:
	Arti3DDevice			*m_pDevice;

	Arti3DVertexBuffer		*m_pVertexBuffer;
	Arti3DIndexBuffer		*m_pIndexBuffer;
};


#endif