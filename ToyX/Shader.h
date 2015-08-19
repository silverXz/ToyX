#pragma once

struct Toy_Vertex;
struct Toy_TransformedVertex;
struct GlobalUniforms;


struct VS_PARAM
{
	Toy_Vertex				*v_in;
	Toy_TransformedVertex	*v_out;
	GlobalUniforms			*uniforms;
};

struct PS_PARAM
{
};

using VertexShader = void(*)(VS_PARAM*);
using FragmentShader = void(*)(PS_PARAM*);

void CubeVS(VS_PARAM* vparm);
void CubeFS(PS_PARAM* pparm);