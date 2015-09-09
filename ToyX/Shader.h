#pragma once

#include <xmmintrin.h>
#include <smmintrin.h>
#include "SSE_Math.h"
#include "ToyMath.h"




#define SSE_ALIGN __declspec(align(16))

struct Toy_Vertex;
struct Arti3DTransformedVertex;
struct GlobalUniforms;




struct VS_PARAM
{
	Toy_Vertex				*v_in;
	Arti3DTransformedVertex	*v_out;
	GlobalUniforms			*uniforms;
};

struct PS_PARAM
{
	SSE_Float Varyings[12];
	SSE_Color3 Output;
	GlobalUniforms		*uniforms;
};

using VertexShader = void(*)(VS_PARAM*);
using FragmentShader = void(*)(PS_PARAM*);

void CubeVS(VS_PARAM* vparm);
void CubeFS(PS_PARAM* pparm);