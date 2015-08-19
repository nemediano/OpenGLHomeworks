#version 150            
uniform mat4 M; 
uniform mat4 PV; 
uniform float time; 
    
in vec3 pos_attrib;  
in vec3 normal_attrib;    
in vec2 tex_coord_attrib; 

out vec2 vtexcoord;
out vec3 vnormal;

void main(void)
{      
	vtexcoord = tex_coord_attrib;
	vnormal = (M*vec4(normal_attrib, 0.0)).xyz;
	gl_Position = PV*M*vec4(pos_attrib, 1.0);
}