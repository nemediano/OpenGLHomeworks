#version 430
//Inputs from vertex shader
in vec2 fPosition;
//Only output for the fragment shader
out vec4 fragcolor;

const vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);

subroutine vec4 fragPattern(vec4 color1, vec4 color2);

vec4 part_1(vec4 color1, vec4 color2);
vec4 part_2(vec4 color1, vec4 color2);
vec4 part_3(vec4 color1, vec4 color2);
vec4 part_4(vec4 color1, vec4 color2);
vec4 part_5(vec4 color1, vec4 color2);
vec4 part_5_5(vec4 color1, vec4 color2);
vec4 part_6(vec4 color1, vec4 color2);
vec4 part_7(vec4 color1, vec4 color2);

subroutine uniform fragPattern patternOption;

void main(void) {
	
	fragcolor = patternOption(red, blue);

}

subroutine (fragPattern) vec4 part_1(vec4 color1, vec4 color2) {
	return vec4(fPosition.x, fPosition.y, 0.0, 1.0);
}

subroutine (fragPattern) vec4 part_2(vec4 color1, vec4 color2) {
	const float edge = 0.5;
	float new_x = step(edge, fPosition.x);
	return mix(color1, color2, new_x);
}

subroutine (fragPattern) vec4 part_3(vec4 color1, vec4 color2) {
	const int num_bars = 10;
	//This makes the range periodical!
	//Since x is in [0, 1], then 2x is in [0, 2]
	//But since we are calculating the fractional part only
	//It goes twice from [0, 1]. Then instead of 2 we make it going 10 times
	float new_x = fract(num_bars * fPosition.x);
	return mix(color1, color2, step(0.5, new_x));
}

subroutine (fragPattern) vec4 part_4(vec4 color1, vec4 color2) {
	const int num_bars = 10;
	const float amplitude = 0.5;
	const float frequency = 40.0;
	float translation_x = amplitude * sin(frequency * fPosition.y);
	float new_x = fract(num_bars * fPosition.x + translation_x);
	return mix(color1, color2, step(0.5, new_x));
}

subroutine (fragPattern) vec4 part_5(vec4 color1, vec4 color2) {
	const vec2 translation = vec2(-0.5, -0.5);
	vec2 new_coordinates = fPosition + translation;
	float r = length(new_coordinates);
	const float radio = 0.25;
	return mix(color1, color2, step(radio, r));	
}

subroutine (fragPattern) vec4 part_5_5(vec4 color1, vec4 color2) {
	const vec2 translation = vec2(-0.5, -0.5);
	vec2 new_coordinates = fPosition + translation;
	float r = length(new_coordinates);
	const float outter_radio = 0.4;
	const float inner_radio = 0.35;
	float mid_point = 0.5 * (outter_radio + inner_radio);
    vec4 color_inside = mix(color1, color2, step(inner_radio, r));
	vec4 color_circle = mix(color2, color1, step(outter_radio, r));
	return mix(color_inside, color_circle, step(mid_point, r));
}

subroutine (fragPattern) vec4 part_6(vec4 color1, vec4 color2) {
	const ivec2 rep_times = ivec2(10, 10);
	vec2 new_coordinates = fract(rep_times * fPosition);
	const vec2 translation = vec2(-0.5, -0.5);
	float r = length(new_coordinates + translation);
	const float relative_radio = 0.35;
	return mix(color1, color2, step(relative_radio, r));
}

subroutine (fragPattern) vec4 part_7(vec4 color1, vec4 color2) {
	const ivec2 rep_times = ivec2(10, 10);
	vec2 new_coordinates = fract(rep_times * fPosition);
	const vec2 translation = vec2(-0.5, -0.5);
	float r = length(new_coordinates + translation);
	const float relative_outter_radio = 0.4;
	const float realtive_inner_radio = 0.3;
	float mid_point = 0.5 * (relative_outter_radio + realtive_inner_radio);
	vec4 color_inside = mix(color1, color2, step(realtive_inner_radio, r));
	vec4 color_circle = mix(color2, color1, step(relative_outter_radio, r));
	return mix(color_inside, color_circle, step(mid_point, r));
}