#version 400

in vec2 Position;
uniform mat4 PVM;


void main(void) { 
	gl_Position = PVM * vec4(Position, 0.0, 1.0);
}
