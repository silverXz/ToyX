#ifndef _ARTI3D_VERTEX_SHADER_H_
#define _ARTI3D_VERTEX_SHADER_H_

#include "Arti3D_Math.h"
#include "Arti3D_Types.h"

class Arti3DDevice;

// Base class for vertex shader.
class Arti3DVertexShader
{
protected:
	Arti3DVertexShader(Arti3DDevice *pDevice);
public:
	virtual ~Arti3DVertexShader();

public:
	virtual void Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput) = 0;

protected:
	Arti3DDevice		*m_pDevice;
};


// Base class for pixel/fragment shaders.
class Arti3DPixelShader
{
protected:
	Arti3DPixelShader(Arti3DDevice *pDevice);
public:
	virtual ~Arti3DPixelShader();

public:
	virtual void Execute(Arti3DPSParam *io_pPSParam) = 0;

protected:
	SSE_Color3	SampleTexture(int iTexUint, SSE_Float& fU, SSE_Float& fV);

protected:
	Arti3DDevice		*m_pDevice;
};


#endif