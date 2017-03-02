#version 430
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TextCoord;
layout (location = 3) in vec3 Color;
layout (location = 4) in mat4 M;

uniform mat4 P;
uniform mat4 V;

out vec3 fNormal;
out vec2 fTextCoord;
out flat vec3 fColor;
out flat int fInstanceId;

void main(void) {
	gl_Position = P * V * M * vec4(Position, 1.0);
	fNormal = vec3(inverse(transpose(V * M)) * vec4(Normal, 0.0));
	fTextCoord = TextCoord;
	fColor = Color;
	fInstanceId = gl_InstanceID;
}
