#version 130
in vec3 fNormal;
in vec2 fTextCoord;

uniform float time;

out vec4 fragcolor;

const vec3 COL_1 = vec3(1.0, 0.0, 0.0);
const vec3 COL_2 = vec3(1.0, 1.0, 0.0);

void main(void) {
	
	vec3 color = mix(COL_1, COL_2, 0.5 * sin(time) + 0.5);
	
	fragcolor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
