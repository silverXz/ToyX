#pragma once

#include <xmmintrin.h>
#include <smmintrin.h>
#include "Arti3D_SSE_Math.h"
#include "Arti3D_ShaderBase.h"

#define SSE_ALIGN __declspec(align(16))

class SimpleCubeVS : public Arti3DVertexShader
{
public:
	SimpleCubeVS(){}
	~SimpleCubeVS() {}

public:
	void Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
};
class SimpleCubePS : public Arti3DPixelShader
{
public:
	SimpleCubePS(){}
	~SimpleCubePS() {}

public:
	void Execute(Arti3DShaderUniform *i_pUnform, Arti3DPSParam *io_pPSParam);
};


class CheckboardCubeVS : public Arti3DVertexShader
{
public:
	CheckboardCubeVS(){}
	~CheckboardCubeVS() {}

public:
	void Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);

};
class CheckboardCubePS : public Arti3DPixelShader
{
public:
	CheckboardCubePS(){}
	~CheckboardCubePS() {}

	void Execute(Arti3DShaderUniform *i_pUnform, Arti3DPSParam *io_pPSParam);
};

class PhongVS : public Arti3DVertexShader
{
public:
	PhongVS(){}
	~PhongVS() {}

	void Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
};

class PhongPS : public Arti3DPixelShader
{
public:
	PhongPS(){}
	~PhongPS() {}

	void Execute(Arti3DShaderUniform *i_pUnform, Arti3DPSParam *io_pPSParam);
};