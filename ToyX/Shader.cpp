#include "stdafx.h"
#include "Shader.h"
#include "ToyRender.h"

using namespace toy;

void CubeVS(VS_PARAM* vparm)
{
	Toy_Vertex				*v_in = vparm->v_in;
	Toy_TransformedVertex	*v_out = vparm->v_out;
	v_out->p = vparm->uniforms->mvp * v_in->p;
	v_out->varyings[0] = v_in->c.x;
	v_out->varyings[1] = v_in->c.y;
	v_out->varyings[2] = v_in->c.z;
}
