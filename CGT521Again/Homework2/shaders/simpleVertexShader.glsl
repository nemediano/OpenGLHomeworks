#version 330            
in vec3 pos_attrib;

void main(void) {
	gl_Position = vec4(pos_attrib, 1.0);
}