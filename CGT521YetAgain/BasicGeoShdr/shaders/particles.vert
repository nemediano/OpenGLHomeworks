#version 430
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Velocity;
layout(location = 2) in float Mass;

layout(location = 0) uniform mat4 PVM;
layout(location = 1) uniform mat4 M;

out VS_OUT {
	vec2 pos;
	float m;
	vec3 color;
} vs_out;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
	vs_out.pos = Position.xy;
	vs_out.color = Velocity;
	vs_out.m = Mass;
}
