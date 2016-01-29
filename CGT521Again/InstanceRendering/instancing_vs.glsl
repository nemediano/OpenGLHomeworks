#version 400            
uniform mat4 PVM;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

flat out int instance_id;

const vec2 deltas = 20.0 * vec2(1.0, 1.0);
const vec3 translation = -20.0 * vec3(1.0, 1.0, 0.0);

void main(void)
{	
	int row = gl_InstanceID % 3;
	int col = gl_InstanceID / 3;
    vec3 new_position = pos_attrib + vec3(row, col, 0.0) * vec3(deltas, 0.0) + translation;
	gl_Position = PVM*vec4(new_position, 1.0);
	instance_id = gl_InstanceID;
}