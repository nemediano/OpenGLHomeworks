#version 330

in vec4 Position;
in vec2 TextCoord;
in vec3 Normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color;

out vec4 vColor;
out vec3 vNormal;
out vec2 vTextCoord;

void main(void) { 
	gl_Position = P * V * M * Position; 
	vNormal = Normal;
	vColor = Color;
	vTextCoord = TextCoord;
}
