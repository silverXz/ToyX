#include "stdafx.h"
#include "Shader.h"
#include "Arti3D_Device.h"
#include "Arti3D_Types.h"

using namespace toy;

void CubeVS(VS_PARAM* vparm)
{
	Toy_Vertex				*v_in = vparm->v_in;
	Arti3DVSOutput	*v_out = vparm->v_out;
	v_out->p = vparm->uniforms->mvp * v_in->p;
	v_out->varyings[0] = v_in->c.x;
	v_out->varyings[1] = v_in->c.y;
	v_out->varyings[2] = v_in->c.z;
} 

void CubeFS(PS_PARAM* pparm)
{
	SSE_Color3 &out = pparm->Output;
	out.r = pparm->Varyings[0];
	out.g = pparm->Varyings[1];
	out.b = pparm->Varyings[2];
}

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
