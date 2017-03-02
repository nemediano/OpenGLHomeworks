#version 430
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TextCoord;

uniform mat4 PVM;

out vec2 fTextCoord;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0);
	fTextCoord = TextCoord;
}
