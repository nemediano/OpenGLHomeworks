#version 130
in vec3 fNormal;
in vec2 fTextCoord;

uniform float time;

out vec4 fragcolor;

const vec3 COL_1 = vec3(1.0, 1.0, 0.5);
const vec3 COL_2 = vec3(1.0, 0.25, 0.0);
const float AMP = 1.2;
const float FRQ = 2.0;

void main(void) {
	
	vec3 color = mix(COL_1, COL_2, AMP * (0.5 * sin(FRQ * (time + 20.0 * (fTextCoord.x * fTextCoord.y)) + 0.5)));
	
	fragcolor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
