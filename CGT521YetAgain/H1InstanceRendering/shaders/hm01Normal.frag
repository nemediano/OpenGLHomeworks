#version 430
in vec3 fNormal;
in vec2 fTextCoord;
flat in vec3 fColor;

out vec4 fragcolor;

void main(void) {
	
	
	vec3 color = vec3(1.0, 0.0, 0.0);
	
	fragcolor = vec4(color, 1.0);
}
