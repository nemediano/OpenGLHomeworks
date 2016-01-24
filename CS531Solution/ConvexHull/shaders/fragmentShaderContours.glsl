#version 400

uniform vec3 Color;

out vec4 fragcolor;

void main(void) {
	fragcolor = vec4(Color, 1.0);
}
