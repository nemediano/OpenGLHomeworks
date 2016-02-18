#version 430

in vec4 Position;
in vec3 Normal;
in vec2 TextureCoordinate;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Outputs for fragment shader
out vec3 fNormal;
out vec3 fPosition;
out vec2 fTextCoord;


void main(void) {
	mat4 PVM = P * V * M;
	mat4 VM = V * M;
	mat4 NormalMatrix = transpose(inverse(VM));
	gl_Position = PVM * Position;
	fNormal = normalize(vec3(NormalMatrix * vec4(Normal, 1.0)));
	//Position needs to be in view space for lighting
	fPosition = vec3(VM * Position);
	fTextCoord = TextureCoordinate;
}
