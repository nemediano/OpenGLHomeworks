#version 400

//Inputs from vertex shader
in vec3 fColor;

out vec4 fragcolor;

void main(void) {
	fragcolor = vec4(fColor, 1.0);
}
