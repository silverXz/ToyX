#pragma once

#include <xmmintrin.h>
#include <smmintrin.h>
#include "Arti3D_SSE_Math.h"




#define SSE_ALIGN __declspec(align(16))

struct Arti3DVSOutput;
struct Arti3DShaderUniform;



struct Arti3DPSParam
{
	SSE_Float Varyings[12];
	SSE_Color3	Output;
};


struct Arti3DVSInput;
struct Arti3DVSOutput;
struct Arti3DShaderUniform;

using Arti3DVertexShader = void(*)(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
using Arti3DPixelShader = void(*)(Arti3DPSParam *io_pPSParam);

void NewCubeVS(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
void NewCubeFS(Arti3DPSParam *io_pPSPram);