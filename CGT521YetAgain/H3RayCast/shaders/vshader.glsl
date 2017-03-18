#version 440            
uniform mat4 M; 
uniform mat4 PV;
uniform mat4 Q;
uniform float time; 
    
in vec3 pos_attrib;      
in vec3 tex_coord_attrib; 

out vec3 vpos;

void main(void)
{      
	gl_Position = PV*M*vec4(pos_attrib, 1.0);
	vpos = (Q*vec4(tex_coord_attrib, 1.0)).xyz;
}