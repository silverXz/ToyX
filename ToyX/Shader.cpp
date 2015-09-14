#include "stdafx.h"
#include "Shader.h"
#include "Arti3D_Device.h"
#include "Arti3D_Types.h"

using namespace a3d;

void NewCubeVS(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput)
{
	o_pVSOutput->p = i_pUniform->mvp * i_pVSInput->ShaderInputs[0];
	o_pVSOutput->varyings[0] = i_pVSInput->ShaderInputs[1].x;
	o_pVSOutput->varyings[1] = i_pVSInput->ShaderInputs[1].y;
	o_pVSOutput->varyings[2] = i_pVSInput->ShaderInputs[1].z;
}

void NewCubeFS(Arti3DPSParam *io_pPSPram)
{
	io_pPSPram->Output.r = io_pPSPram->Varyings[0];
	io_pPSPram->Output.g = io_pPSPram->Varyings[1];
	io_pPSPram->Output.b = io_pPSPram->Varyings[2];
}
