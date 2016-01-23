#version 400

in vec2 Position;
in vec3 Color;

uniform mat4 PVM;

out vec3 fColor;

void main(void) { 
	gl_Position = PVM * vec4(Position, 0.0, 1.0);
	gl_PointSize = 5.0;
	fColor = Color;
}
