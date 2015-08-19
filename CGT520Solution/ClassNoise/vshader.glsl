#version 150            
uniform mat4 VM; 
uniform mat4 P;  
    
in vec3 pos_attrib;   
in vec2 tex_coord_attrib; 

out vec2 vtexcoord;
out vec3 vpos;

void main(void)
{      
	vpos = pos_attrib;
	vtexcoord = tex_coord_attrib;
	gl_Position = P*VM*vec4(pos_attrib, 1.0);
}