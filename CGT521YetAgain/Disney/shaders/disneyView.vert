#version 430
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextCoord;

layout(location = 0) uniform mat4 PVM;
layout(location = 1) uniform mat4 VM;
layout(location = 2) uniform mat4 NormalMat;

out vec3 fNormal;
out vec2 fTextCoord;
out vec3 fPosition;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
	fNormal = (NormalMat * vec4(Normal, 0.0)).xyz;
	fPosition = (VM * vec4(Position, 1.0f)).xyz;
	fTextCoord = TextCoord;
}
