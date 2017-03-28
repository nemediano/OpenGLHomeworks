#version 430
in vec3 Position;
in vec3 Normal;
in vec2 TextCoord;

uniform mat4 PVM;

out vec3 fNormal;
out vec2 fTextCoord;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
	fNormal = Normal;
	fTextCoord = TextCoord;
}
