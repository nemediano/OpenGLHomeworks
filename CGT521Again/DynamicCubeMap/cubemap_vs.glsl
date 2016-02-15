#version 400
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec3 pos_attrib;
in vec3 normal_attrib;


out vec3 normal_v;
out vec3 view_v;


void main(void)
{
	gl_Position = P * V * M * vec4(pos_attrib, 1.0);

	normal_v = normal_attrib;
	view_v = vec4(V * M * vec4(pos_attrib, 1.0)).xyz;
}