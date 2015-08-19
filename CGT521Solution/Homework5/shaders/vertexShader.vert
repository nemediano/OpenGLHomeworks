#version 150
uniform float time;
uniform int fish_animation;
uniform int texture_map_flag;
uniform int texture_animation;
uniform int special_animation;
uniform mat4 VM;
uniform mat4 P;
in vec3 pos_attrib;
in vec2 tex_coord_attrib;
out vec2 text_coord;
void main(void){
	vec4 position;
	position = vec4(pos_attrib, 1.0);
	text_coord = tex_coord_attrib;
	if (fish_animation != 0) {
		position.z += sin(3.0 * (time + 2.8 * position.x)) / 32.0;
	} else if (special_animation != 0) {
		position.y *= 0.5 * sin(3.7 * (time + position.x)) + 2;
		position.z *= 0.5 * sin(3.7 * (time + position.x)) + 2;
	}
	gl_Position = P * VM * position;
}
