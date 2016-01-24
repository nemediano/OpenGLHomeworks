#version 400

//Uniforms
uniform vec3 Color;

out vec4 fragcolor;

void main(void) {
	fragcolor = vec4(Color, 1.0);
}
