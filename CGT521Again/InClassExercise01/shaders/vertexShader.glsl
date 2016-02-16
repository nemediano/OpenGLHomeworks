#version 430

in vec2 Position;

uniform mat4 PVM;

out vec2 fPosition;

void main(void) { 
	gl_Position = PVM * vec4(Position.x, Position.y, 0.0, 1.0f);
	const float scale = 0.5;
	const vec2 center = vec2(0.5, 0.5);
	fPosition = scale * Position + center;
}
