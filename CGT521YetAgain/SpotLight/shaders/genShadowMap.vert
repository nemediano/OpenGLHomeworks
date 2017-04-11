#version 430
layout(location = 0) in vec3 Position;
layout(location = 0) uniform mat4 PVM;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
}
