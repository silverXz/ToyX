#include "stdafx.h"
#include "Shader.h"
#include "Arti3D_Device.h"
#include "Arti3D_Types.h"


// Implementation of SimpleCube Shader.....................
void SimpleCubeVS::Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput)
{
	o_pVSOutput->p = i_pUniform->mvp * i_pVSInput->ShaderInputs[0];
	o_pVSOutput->varyings[0] = i_pVSInput->ShaderInputs[1].x;
	o_pVSOutput->varyings[1] = i_pVSInput->ShaderInputs[1].y;
	o_pVSOutput->varyings[2] = i_pVSInput->ShaderInputs[1].z;
}
void SimpleCubePS::Execute(Arti3DPSParam *io_pPSParam)
{
	io_pPSParam->Output.r = io_pPSParam->Varyings[0];
	io_pPSParam->Output.g = io_pPSParam->Varyings[1];
	io_pPSParam->Output.b = io_pPSParam->Varyings[2];
}


// Implementation of CheckboardCube Shaders...................
void CheckboardCubeVS::Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput)
{
	// Pass through vertex position.
	o_pVSOutput->p = i_pUniform->mvp * i_pVSInput->ShaderInputs[0];

	// Pass through vertex texture coordinates.
	o_pVSOutput->varyings[0] = i_pVSInput->ShaderInputs[1].x;
	o_pVSOutput->varyings[1] = i_pVSInput->ShaderInputs[1].y;
}

void CheckboardCubePS::Execute(Arti3DPSParam *io_pPSParam)
{
	io_pPSParam->Output = SampleTexture(0,io_pPSParam->Varyings[0], io_pPSParam->Varyings[1]);
}
