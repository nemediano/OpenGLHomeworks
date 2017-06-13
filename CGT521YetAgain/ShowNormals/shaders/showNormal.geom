#version 430
layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform mat4 PV;
uniform float displacement;
uniform vec3 color;

in VS_OUT {
	vec3 position;
	vec3 normal;
} gs_in[];

out GS_OUT {
	vec3 color;
} gs_out;

void main(void) {
	vec4 newPos = vec4(0.0);
	vec4 dis = displacement * vec4(normalize(gs_in[0].normal), 0.0);
	
	newPos = vec4(gs_in[0].position, 1.0);
	gs_out.color = color;
	gl_Position = PV * newPos;
	EmitVertex();
	
	newPos = vec4(gs_in[0].position, 1.0) + dis;
	gs_out.color = color;
	gl_Position = PV * newPos;
	EmitVertex();
	
	EndPrimitive();
}
