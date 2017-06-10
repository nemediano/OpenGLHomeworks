#version 430
in vec3 c;

uniform float gamma = 1.0;
out vec4 fragcolor;

vec3 gammaCorrection(vec3 color);
vec3 toneMapping(vec3 color);

const float EPSILON = 1e-7;
const float PI = 3.14159;

void main(void) {
	vec3 color = vec3(0.0);
	color = c;
	
	fragcolor = vec4(gammaCorrection(color), 1.0);
}

vec3 gammaCorrection(vec3 color) {
	return pow(color, vec3(1.0 / gamma));
}

vec3 toneMapping(vec3 color) {
	return color / (color + vec3(1.0));
}