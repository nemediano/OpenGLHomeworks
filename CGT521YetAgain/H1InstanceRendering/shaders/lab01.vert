#version 430
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TextCoord;
layout (location = 3) in vec3 Color;
//layout (location = 4) in mat4 M;

uniform mat4 PV;
uniform mat4 M;

out vec3 fNormal;
out vec2 fTextCoord;
flat out vec3 fColor;

void main(void) {
	gl_Position = PV * M * vec4(Position, 1.0);
	fNormal = Normal;
	fTextCoord = TextCoord;
	fColor = Color;
}
