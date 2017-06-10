#version 430
layout(points) in;
layout(triangle_strip, max_vertices = 10) out;

const float TAU = 6.28318;
const float EPSILON = 1e-2;

layout(location = 0) uniform mat4 PVM;

in VS_OUT {
    vec2 pos;
	float m;
	vec3 color;
} gs_in[];

out vec3 c;

void main(void) {
		
	//This needs to be done before the first vertex is emited
	c = gs_in[0].color;
	const float deltaAngle = TAU / 10.0;
	const float strideAngle = TAU / 4.0;
	float angle = 0.0;
	float minRad = 0.5 * gs_in[0].m;
	float maxRad = gs_in[0].m;
	//First normal triangle
	angle = 5.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + (minRad - EPSILON) * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();   
	angle = 6.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + maxRad * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();
	angle = 7.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + (minRad - EPSILON) * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();
    EndPrimitive();
	
	//Second normal triangle
	angle = 0.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + maxRad * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();   
	angle = 1.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + (minRad - EPSILON) * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();
	angle = 9.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + (minRad - EPSILON) * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();
    EndPrimitive();
	
	//Triangle strip
	angle = 2.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + maxRad * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();   
	angle = 3.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + minRad * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();
	angle = 8.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + maxRad * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();
	angle = 4.0 * deltaAngle + strideAngle;
	gl_Position = PVM * vec4(gs_in[0].pos + maxRad * vec2(cos(angle), sin(angle)), 0.0, 1.0);
	EmitVertex();
    EndPrimitive();
}
