#version 150
in vec4 vPosition;
uniform mat4 M1;
uniform mat4 M2;
uniform mat4 M3;
uniform vec3 vColor;
out vec4 color;

void main(void){
	gl_Position = M3 * M2 * M1 * vPosition;
	color = vec4(vColor, 1.0f);
}
