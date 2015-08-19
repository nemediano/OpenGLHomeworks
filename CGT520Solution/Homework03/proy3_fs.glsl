#version 330

uniform int normColorOpt;

in vec4 vColor;
in vec4 vNormal;

out vec4 fragcolor;
void main(void) {
	fragcolor = mix(vColor, vNormal, normColorOpt);
}
