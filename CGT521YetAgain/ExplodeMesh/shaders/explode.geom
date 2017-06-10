#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//layout(location = 0) uniform mat4 PV;

in VS_OUT {
	vec3 normal;
	vec2 textCoord;
	vec3 position;
} gs_in[];


out GS_OUT {
	vec3 normal;
	vec2 textCoord;
	vec3 position;
} gs_out;

void main(void) {
	gs_out.normal = gs_in[0].normal;
	gs_out.textCoord = gs_in[0].textCoord;
	gs_out.position = gs_in[0].position;
	EmitVertex();
	
	gs_out.normal = gs_in[1].normal;
	gs_out.textCoord = gs_in[1].textCoord;
	gs_out.position = gs_in[1].position;
	EmitVertex();
	
	gs_out.normal = gs_in[2].normal;
	gs_out.textCoord = gs_in[2].textCoord;
	gs_out.position = gs_in[2].position;
	EmitVertex();
	
	EndPrimitive();
}
