#version 150            
uniform mat4 M; 
uniform mat4 PV;  
    
in vec3 pos_attrib;   
in vec2 tex_coord_attrib; 

out vec2 vtexcoord;

void main(void)
{      
	vtexcoord = tex_coord_attrib;
	gl_Position = PV*M*vec4(pos_attrib, 1.0);
}