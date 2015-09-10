#pragma once

#include <xmmintrin.h>
#include <smmintrin.h>
#include "SSE_Math.h"
#include "ToyMath.h"




#define SSE_ALIGN __declspec(align(16))

struct Toy_Vertex;
struct Arti3DVSOutput;
struct Arti3DShaderUniform;




struct VS_PARAM
{
	Toy_Vertex				*v_in;
	Arti3DVSOutput	*v_out;
	Arti3DShaderUniform			*uniforms;
};

struct PS_PARAM
{
	SSE_Float Varyings[12];
	SSE_Color3 Output;
	Arti3DShaderUniform		*uniforms;
};

struct Arti3DPSParam
{
	SSE_Float Varyings[12];
	SSE_Color3	Output;
};

using VertexShader = void(*)(VS_PARAM*);
using FragmentShader = void(*)(PS_PARAM*);

struct Arti3DVSInput;
struct Arti3DVSOutput;
struct Arti3DShaderUniform;

using Arti3DVertexShader = void(*)(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
using Arti3DPixelShader = void(*)(Arti3DPSParam *io_pPSParam);

void CubeVS(VS_PARAM* vparm);
void CubeFS(PS_PARAM* pparm);

void NewCubeVS(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
void NewCubeFS(Arti3DPSParam *io_pPSPram);