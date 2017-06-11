#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 PV;
uniform float time;
uniform float amplitude;
uniform float frequency;

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

vec3 calculateNormal();

void main(void) {
	
	vec4 newPos = vec4(0.0);
	float stepSize = amplitude * (0.5 * sin(frequency * time) + 0.5);
	vec4 displacement = stepSize * vec4(calculateNormal(), 0.0);
	
	gs_out.normal = gs_in[0].normal;
	gs_out.textCoord = gs_in[0].textCoord;
	newPos = vec4(gs_in[0].position, 1.0) + displacement;
	gs_out.position = newPos.xyz;
	gl_Position = PV * newPos;
	EmitVertex();
	
	gs_out.normal = gs_in[1].normal;
	gs_out.textCoord = gs_in[1].textCoord;
	newPos = vec4(gs_in[1].position, 1.0) + displacement;
	gs_out.position = newPos.xyz;
	gl_Position = PV * newPos;
	EmitVertex();
	
	gs_out.normal = gs_in[2].normal;
	gs_out.textCoord = gs_in[2].textCoord;
	newPos = vec4(gs_in[2].position, 1.0) + displacement;
	gs_out.position = newPos.xyz;
	gl_Position = PV * newPos;
	EmitVertex();
	
	EndPrimitive();
}

vec3 calculateNormal() {
   vec3 u = vec3(gs_in[2].position) - vec3(gs_in[1].position);
   vec3 v = vec3(gs_in[0].position) - vec3(gs_in[1].position);
   return normalize(cross(u, v));
}  
