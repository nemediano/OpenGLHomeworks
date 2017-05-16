#version 430
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TextCoord;

layout(location = 0) uniform mat4 PVM;
layout(location = 1) uniform mat4 M;

out vec2 fTextCoord;
out vec3 fPosition;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
	fPosition = (M * vec4(Position, 1.0f)).xyz;
	fTextCoord = TextCoord;
}
