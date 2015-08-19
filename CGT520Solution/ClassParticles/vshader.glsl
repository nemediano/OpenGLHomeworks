#version 400            
uniform mat4 M; 
uniform mat4 PV; 
uniform float time; 
    
in vec3 pos_attrib;
in vec4 color_attrib;
in float size_attrib;

out vec4 vcolor;

void main(void)
{      
	gl_PointSize = size_attrib;
	vcolor = color_attrib;
	gl_Position = PV * M * vec4(pos_attrib, 1.0);
}