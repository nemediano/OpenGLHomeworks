#version 430
layout(location = 0) in vec3 Position;
layout(location = 0) uniform mat4 PVM;

out vec4 position;

void main(void) {
	position = PVM * vec4(Position, 1.0f);
	gl_Position = position;
}
