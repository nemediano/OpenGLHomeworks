#version 430
in vec3 Position;
in vec3 Normal;

uniform mat4 PVM;
uniform mat4 M;
uniform mat4 NormMat;

//Output for fragement shader is in world space
out vec3 fNormal;
out vec3 fPosition;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
	fPosition = (M * vec4(Position, 1.0f)).xyz;
	fNormal = (NormMat * vec4(Normal, 0.0)).xyz;
}
