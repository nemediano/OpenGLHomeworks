#version 430
layout(location = 0) in vec3 Position;

layout(location = 0) uniform mat4 PVM;
layout(location = 1) uniform mat4 M;

out vec3 fTextCoord;

void main(void) {
	gl_Position = PVM * vec4(Position, 1.0f);
	//Weird situation, corrdinates inside the box are in left hand coordinate system
	//Plus OpenGl invert images vertical, so we need to invert both.
	fTextCoord = vec3(-Position.x, -Position.y, Position.z);
}
