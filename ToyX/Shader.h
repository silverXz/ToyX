#pragma once

#include <xmmintrin.h>
#include <smmintrin.h>
#include "Arti3D_SSE_Math.h"
#include "Arti3D_ShaderBase.h"

#define SSE_ALIGN __declspec(align(16))

class SimpleCubeVS : public Arti3DVertexShader
{
public:
	SimpleCubeVS(Arti3DDevice *pDevice) : Arti3DVertexShader(pDevice) {}
	~SimpleCubeVS() {}

public:
	void Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
};
class SimpleCubePS : public Arti3DPixelShader
{
public:
	SimpleCubePS(Arti3DDevice *pDevice) : Arti3DPixelShader(pDevice) {}
	~SimpleCubePS() {}

public:
	void Execute(Arti3DPSParam *io_pPSParam);
};


class CheckboardCubeVS : public Arti3DVertexShader
{
public:
	CheckboardCubeVS(Arti3DDevice *pDevice) : Arti3DVertexShader(pDevice) {}
	~CheckboardCubeVS() {}

public:
	void Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);

};
class CheckboardCubePS : public Arti3DPixelShader
{
public:
	CheckboardCubePS(Arti3DDevice *pDevice) : Arti3DPixelShader(pDevice) {}
	~CheckboardCubePS() {}

	void Execute(Arti3DPSParam *io_pPSParam);
};

// struct Arti3DVSInput;
// struct Arti3DVSOutput;
// struct Arti3DShaderUniform;
// 
// //using Arti3DVertexShader = void(*)(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
// //using Arti3DPixelShader = void(*)(Arti3DPSParam *io_pPSParam);
// 
// void NewCubeVS(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
// void NewCubeFS(Arti3DPSParam *io_pPSPram);
// 
// void CheckboardCubeVS(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform *i_pUniform, Arti3DVSOutput *o_pVSOutput);
