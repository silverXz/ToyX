#ifndef _ARTI3D_VERTEX_SHADER_H_
#define _ARTI3D_VERTEX_SHADER_H_

#include "Arti3D_Math.h"
#include "Arti3D_Types.h"

class Arti3DDevice;

typedef a3d::vec4 ShaderRegister;

struct Arti3DVSInput {
	ShaderRegister ShaderInputs[ARTI3D_MAX_VERTEX_SHADER_REGISTER];
};

struct Arti3DVSOutput
{
	a3d::vec4 p;						// Vertex position.
	float varyings[ARTI3D_MAX_VARYING];	// Other vertex attributes.
};

struct Arti3DVertexCache
{
	uint32_t				tag;			// Cache Index.
	Arti3DVSOutput			vs_output;		// Cached Information.

	Arti3DVertexCache() : tag(UINT_MAX){}

	inline void Clear()
	{
		tag = UINT_MAX;
	}
};


// Base class for vertex shader.
class Arti3DVertexShader
{
public:
	Arti3DVertexShader(Arti3DDevice *pDevice);
	virtual ~Arti3DVertexShader();

public:
	virtual void Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput) = 0;
	void Use();
protected:
	Arti3DDevice		*m_pDevice;
};


struct Arti3DPSParam
{
	SSE_Float Varyings[12];
	SSE_Color3	Output;
};


// Base class for pixel/fragment shaders.
class Arti3DPixelShader
{
public:
	Arti3DPixelShader(Arti3DDevice *pDevice);
	virtual ~Arti3DPixelShader();

public:
	virtual void Execute(Arti3DPSParam *io_pPSParam) = 0;
	void Use();

protected:
	SSE_Color3	SampleTexture(int iTexUint, SSE_Float& fU, SSE_Float& fV);

protected:
	Arti3DDevice		*m_pDevice;
};


#endif