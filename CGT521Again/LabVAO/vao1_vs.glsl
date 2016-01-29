#version 400            

in vec3 pos_attrib;
out vec2 scaled_position;
void main(void)
{
	gl_Position = vec4(pos_attrib, 1.0);
	scaled_position = pos_attrib.xy + vec2(1.0, 1.0);
}