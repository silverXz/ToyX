#include "stdafx.h"
#include "Shader.h"
#include "Arti3D_Device.h"
#include "Arti3D_Types.h"


// Implementation of SimpleCube Shader.....................
void SimpleCubeVS::Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput)
{
	// Similar with gl_Position.
	o_pVSOutput->p = i_pUniform->mvp * i_pVSInput->ShaderInputs[0];
	o_pVSOutput->varyings[0] = i_pVSInput->ShaderInputs[1].x;
	o_pVSOutput->varyings[1] = i_pVSInput->ShaderInputs[1].y;
	o_pVSOutput->varyings[2] = i_pVSInput->ShaderInputs[1].z;
}
void SimpleCubePS::Execute(Arti3DShaderUniform*, Arti3DPSParam *io_pPSParam)
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

void CheckboardCubePS::Execute(Arti3DShaderUniform *i_pUniform, Arti3DPSParam *io_pPSParam)
{
	Arti3DSurface *pSurface = i_pUniform->pSurfaces[0];
	io_pPSParam->Output = SampleTexture(pSurface,io_pPSParam->Varyings[0], io_pPSParam->Varyings[1]);
}

void PhongVS::Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput)
{
	// gl_Position
	o_pVSOutput->p = i_pUniform->mvp * i_pVSInput->ShaderInputs[0];

	a3d::mat4 mv = i_pUniform->view * i_pUniform->model;
	
	// Calculate position in camera-space
	a3d::vec4 vPosInCam = mv * i_pVSInput->ShaderInputs[0];
	o_pVSOutput->varyings[0] = vPosInCam.x;
	o_pVSOutput->varyings[1] = vPosInCam.y;
	o_pVSOutput->varyings[2] = vPosInCam.z;
	
	// Calculate normal in camera-space.
	a3d::vec3 vNormInCam = a3d::mat3(mv) * a3d::vec3(i_pVSInput->ShaderInputs[1]);
	o_pVSOutput->varyings[3] = vNormInCam.x;
	o_pVSOutput->varyings[4] = vNormInCam.y;
	o_pVSOutput->varyings[5] = vNormInCam.z;
}

void PhongPS::Execute(Arti3DShaderUniform *i_pUnform, Arti3DPSParam *io_pPSParam)
{
	// Fetch Vertex Position.
	SSE_Vec3 *pVertexPos= reinterpret_cast<SSE_Vec3*>(&io_pPSParam->Varyings[0]);
	
	// Light Direction.
	const a3d::vec3 &lp_world = i_pUnform->lights[0].vPosition;
	const a3d::vec3 &li = i_pUnform->lights[0].vIntensity;
	a3d::vec3 lp = a3d::vec3(i_pUnform->view * a3d::vec4(lp_world, 1.0f));

	SSE_Vec3 vLightPosition = SSE_Vec3(SSE_Float(lp.x),SSE_Float(lp.y),SSE_Float(lp.z));
	SSE_Vec3 vLightIntensity = SSE_Vec3(SSE_Float(li.x),SSE_Float(li.y),SSE_Float(li.z));

	SSE_Vec3 vLightDir = vLightPosition - *pVertexPos;
	vLightDir.Normalize();

	// Eye Direction.
	SSE_Vec3 vEyeDir = -(*pVertexPos);
	vEyeDir.Normalize();

	// Normal/
	SSE_Vec3 *pNormal = reinterpret_cast<SSE_Vec3*>(&io_pPSParam->Varyings[3]);	
	pNormal->Normalize();

	// Light Reflection Direction.
	SSE_Vec3 vReflect = SSE_Reflect(-vLightDir, *pNormal);

	SSE_Float fDiffuse	= SSE_Max(vReflect.Dot(*pNormal), SSE_ZERO);
	SSE_Float fSpec		= SSE_Max(vReflect.Dot(vEyeDir),  SSE_ZERO);

	// No power instructions in SSE. So...
	fSpec *= fSpec;
	fSpec *= fSpec;
	fSpec *= fSpec;

	const a3d::vec3 va = i_pUnform->material.vAmbient;
	const a3d::vec3 vd = i_pUnform->material.vDiffuse;
	const a3d::vec3 vs = i_pUnform->material.vSpecular;

	SSE_Vec3 vAmbient	= SSE_Vec3(SSE_Float(va.x), SSE_Float(va.y), SSE_Float(va.z));
	SSE_Vec3 vDiffuse	= SSE_Vec3(SSE_Float(vd.x), SSE_Float(vd.y), SSE_Float(vd.z)) * fDiffuse;
	SSE_Vec3 vSpecular	= SSE_Vec3(SSE_Float(vs.x), SSE_Float(vs.y), SSE_Float(vs.z)) * fSpec;

	io_pPSParam->Output = vLightIntensity * (vAmbient + vDiffuse + vSpecular);
}

void SkyboxVS::Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput)
{
	// The input is already in NDC.
	o_pVSOutput->p = i_pVSInput->ShaderInputs[0];

	// Generate view direction for this vetex.
	a3d::vec3 viewDir = a3d::vec3(o_pVSOutput->p.x, o_pVSOutput->p.y, o_pVSOutput->p.z);
	viewDir = a3d::normalize(viewDir);

	o_pVSOutput->varyings[0] = viewDir.x;
	o_pVSOutput->varyings[1] = viewDir.y;
	o_pVSOutput->varyings[2] = viewDir.z;
}

void SkyboxPS::Execute(Arti3DShaderUniform *i_pUniform, Arti3DPSParam *io_pPSParam)
{
	Arti3DCubeTexture *pCubeTexture = i_pUniform->pCubeTextures[0];
	io_pPSParam->Output = SampleCubeTexture(pCubeTexture, io_pPSParam->Varyings[0], io_pPSParam->Varyings[1],io_pPSParam->Varyings[2]);
}
