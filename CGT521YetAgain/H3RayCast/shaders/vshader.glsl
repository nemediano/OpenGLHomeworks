#version 440            

layout(location = 0) uniform mat4 PVM;
layout(location = 1) uniform mat4 Q;
    
layout(location = 0) in vec3 pos_attrib;      
layout(location = 1) in vec3 tex_coord_attrib; 

out vec3 vpos;

void main(void) {      
	gl_Position = PVM * vec4(pos_attrib, 1.0);
	vpos = (Q * vec4(tex_coord_attrib, 1.0)).xyz;
}
