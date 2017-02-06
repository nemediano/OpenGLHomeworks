#version 430
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TextCoord;

uniform mat4 PVM;
uniform mat4 NormMat;
uniform vec3 Color;

out vec3 fNormal;
out vec2 fTextCoord;
flat out vec3 fColor;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0);
	fNormal = vec3(NormMat * vec4(Normal, 0.0));
	fTextCoord = TextCoord;
	fColor = Color;
}
