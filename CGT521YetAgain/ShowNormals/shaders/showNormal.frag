#version 430
in GS_OUT {
	vec3 color;
} fs_in;

uniform float gamma;

out vec4 fragcolor;

vec3 gammaCorrection(vec3 color);
vec3 toneMapping(vec3 color);

void main(void) {
	fragcolor = vec4(gammaCorrection(fs_in.color), 1.0);
}

vec3 gammaCorrection(vec3 color) {
	return pow(color, vec3(1.0 / gamma));
}

vec3 toneMapping(vec3 color) {
	return color / (color + vec3(1.0));
}