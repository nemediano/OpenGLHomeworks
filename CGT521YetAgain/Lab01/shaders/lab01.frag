#version 140
in vec3 fNormal;
in vec2 fTextCoord;
flat in int finstanceID;

uniform float time;

out vec4 fragcolor;

const int NCLRS = 3;


vec3 colors[NCLRS] = vec3[](vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

void main(void) {
	fragcolor = vec4(colors[finstanceID % NCLRS], 1.0);
}
