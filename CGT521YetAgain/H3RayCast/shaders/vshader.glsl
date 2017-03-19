#version 440            

uniform mat4 PVM;
uniform mat4 Q;
    
in vec3 pos_attrib;      
in vec3 tex_coord_attrib; 

out vec3 vpos;

void main(void) {      
	gl_Position = PVM * vec4(pos_attrib, 1.0);
	vpos = (Q * vec4(tex_coord_attrib, 1.0)).xyz;
}
