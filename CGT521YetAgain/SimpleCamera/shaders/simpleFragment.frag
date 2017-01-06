#version 130

in vec3 fNormal;
in vec2 fTextCoord;

uniform sampler2D texture_image;

out vec4 fragcolor;

void main(void) {
	fragcolor = vec4(texture(texture_image, fTextCoord).rgb, 1.0);
}
