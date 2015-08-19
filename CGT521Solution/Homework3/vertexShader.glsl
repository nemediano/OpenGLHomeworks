#version 150
uniform float time;
uniform int fish_animation;
uniform int texture_map_flag;
uniform int texture_animation;
uniform mat4 VM;
uniform mat4 P;
in vec3 pos_attrib;
in vec2 tex_coord_attrib;
out vec2 text_coord;

void main(void){
	vec4 position;
	position = vec4(pos_attrib, 1.0);
	text_coord = tex_coord_attrib;
	float amplitude = 0.05f;
	float frequency = 4.0f;
	if (fish_animation != 0) {
		position.z = amplitude * sin(frequency * (time + position.x));
	}
	gl_Position = P * VM * position;
}
