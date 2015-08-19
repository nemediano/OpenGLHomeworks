#version 150
uniform float time;
uniform int fish_animation;
uniform int texture_map_flag;
uniform int texture_animation;
in vec2 text_coord;
uniform sampler2D texture_map;
out vec4 fragcolor;
void main(void) {
	if (texture_map_flag != 0) {
		fragcolor = texture(texture_map, text_coord);
	} else {
		fragcolor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}
