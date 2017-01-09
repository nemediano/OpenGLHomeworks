#version 430
in vec3 Position;
in vec3 Normal;
in vec2 TextCoord;

uniform mat4 PVM;
uniform mat4 M;
uniform mat4 NormMat;

//Output is in world space
out vec3 fNormal;
out vec2 fTextCoord;
out vec3 fPosition;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
	fPosition = (M * vec4(Position, 1.0f)).xyz;
	fNormal = (NormMat * vec4(Normal, 0.0)).xyz;
	fTextCoord = TextCoord;
}
