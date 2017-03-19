#version 440

sample in vec3 vpos;
out vec4 fragcolor;   

void main(void) {   
	fragcolor = vec4(vpos, 1.0);
}
