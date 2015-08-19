#version 150            
uniform mat4 VM; 
uniform mat4 P;  

in vec3 pos_attrib;   
in vec3 normal_attrib;  
in vec2 tex_coord_attrib; 

out vec3 vnormal;
out vec3 vview;
out vec2 vtextcoord;

void main(void) {      
	vnormal = (VM * vec4(normal_attrib, 0.0f)).xyz;		//eye-space normal
	vview = -(VM * vec4(pos_attrib, 1.0)).xyz;		//eye space view
	vtextcoord = tex_coord_attrib;
	gl_Position = P * VM * vec4(pos_attrib, 1.0);
}