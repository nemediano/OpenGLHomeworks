#version 430
layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

const float TAU = 6.28318;

layout(location = 0) uniform mat4 PVM;

in VS_OUT {
    vec2 pos;
	float m;
	vec3 color;
} gs_in[];

out vec3 c;

void main(void) {
	const int N = 3;
	
	//This needs to be done before the first vertex is emited
	c = gs_in[0].color;
	
	float angle = 0.0;
	float deltaAngle = TAU / N;
	
	for (int i = 0; i < N; ++i) {
		gl_Position = PVM * vec4(gs_in[0].pos + gs_in[0].m * vec2(cos(angle), sin(angle)), 0.0, 1.0);
		angle += deltaAngle;
		EmitVertex();   
	}
	
    EndPrimitive();
}
