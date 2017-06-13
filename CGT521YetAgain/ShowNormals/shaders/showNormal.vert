#version 430
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

layout(location = 0) uniform mat4 PVM;
layout(location = 1) uniform mat4 M;
layout(location = 2) uniform mat4 NormalMat;

out VS_OUT {
	vec3 position;
	vec3 normal;
} vs_out;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
	vs_out.normal = (NormalMat * vec4(Normal, 0.0)).xyz;
	vs_out.position = (M * vec4(Position, 1.0f)).xyz;
}
