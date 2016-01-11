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

class PhongVS : public Arti3DVertexShader
{
public:
	PhongVS(Arti3DDevice *pDevice) : Arti3DVertexShader(pDevice) {}
	~PhongVS() {}

	void Execute(Arti3DVSInput *i_pVSInput, Arti3DShaderUniform* i_pUniform, Arti3DVSOutput *o_pVSOutput);
};

class PhongPS : public Arti3DPixelShader
{
public:
	PhongPS(Arti3DDevice *pDevice) : Arti3DPixelShader(pDevice) {}
	~PhongPS() {}

	void Execute(Arti3DPSParam *io_pPSParam);

	void SetLightPosition(const a3d::vec3& vLight);
	void SetLightIntensity(const a3d::vec3& vIntensity);
	void SetAmbient(const a3d::vec3& vAmbient);
	void SetDiffuse(const a3d::vec3& vDiffuse);
	void SetSpecular(const a3d::vec3& vSpecular);
	void SetShinness(float fShinness);

private:

	SSE_Vec3	vLightPosition;
	SSE_Vec3	vLightIntensity;
	SSE_Vec3	vKa;
	SSE_Vec3	vKd;
	SSE_Vec3	vKs;
	SSE_Float	fShinness;
};