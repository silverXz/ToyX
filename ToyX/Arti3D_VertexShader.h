#ifndef _ARTI3D_VERTEX_SHADER_H_
#define _ARTI3D_VERTEX_SHADER_H_

#include "Arti3D_Math.h"

class Arti3DDevice;

class Arti3DVertexShader
{
protected:
	friend class Arti3DDevice;
	Arti3DVertexShader(Arti3DDevice *pDevice);
	~Arti3DVertexShader();

public:
	void Execute();

private:
	Arti3DDevice		*m_pDevice;
};


#endif