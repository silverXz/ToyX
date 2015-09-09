#include "stdafx.h"
#include "Shader.h"
#include "Arti3D_Device.h"

using namespace toy;

void CubeVS(VS_PARAM* vparm)
{
	Toy_Vertex				*v_in = vparm->v_in;
	Arti3DTransformedVertex	*v_out = vparm->v_out;
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
