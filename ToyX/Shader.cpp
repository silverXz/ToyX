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

void PhongPS::Execute(Arti3DPSParam *io_pPSParam)
{
	// Fetch Vertex Position.
	SSE_Vec3 *pVertexPos= reinterpret_cast<SSE_Vec3*>(&io_pPSParam->Varyings[0]);
	
	// Light Direction.
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
	fSpec = fSpec * fSpec * fSpec * fSpec * fSpec * fSpec;  

	SSE_Vec3 vAmbient	= vKa;
	SSE_Vec3 vDiffuse	= vKd * fDiffuse;
	SSE_Vec3 vSpecular	= vKs * fSpec;

	io_pPSParam->Output = vLightIntensity * (vKa + 
		vKd * SSE_Max(vReflect.Dot(*pNormal),SSE_ZERO) + 
		vKs * vSpecular);
}

void PhongPS::SetLightPosition(const a3d::vec3& vLight)
{
	vLightPosition = SSE_Vec3(SSE_Float(vLightPosition.x),SSE_Float(vLightPosition.y),SSE_Float(vLightPosition.z));
}

void PhongPS::SetLightIntensity(const a3d::vec3& vIntensity)
{
	vLightIntensity = SSE_Vec3(SSE_Float(vIntensity.x), SSE_Float(vIntensity.y), SSE_Float(vIntensity.z));
}

void PhongPS::SetAmbient(const a3d::vec3& vAmbient)
{
	vKa = SSE_Vec3(SSE_Float(vAmbient.x), SSE_Float(vAmbient.y), SSE_Float(vAmbient.z));
}

void PhongPS::SetDiffuse(const a3d::vec3& vDiffuse)
{
	vKd = SSE_Vec3(SSE_Float(vDiffuse.x), SSE_Float(vDiffuse.y), SSE_Float(vDiffuse.z));
}

void PhongPS::SetSpecular(const a3d::vec3& vSpecular)
{
	vKs = SSE_Vec3(SSE_Float(vSpecular.x), SSE_Float(vSpecular.y), SSE_Float(vSpecular.z));
}

void PhongPS::SetShinness(float fShinness)
{
	this->fShinness = SSE_Float(fShinness);
}
