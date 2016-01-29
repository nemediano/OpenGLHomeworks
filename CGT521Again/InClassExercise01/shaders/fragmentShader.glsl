#version 330
//Inputs from vertex shader
in vec2 fPosition;
//Only output for the fragment shader
out vec4 fragcolor;

const vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);

void main(void) {
	// part1
	//fragcolor = vec4(fPosition.x, fPosition.y, 0.0, 1.0);
	
	//part2
	//const float edge = 0.5;
	//float new_x = step(edge, fPosition.x);
	//fragcolor = mix(red, blue, new_x);
	
	//part3
	//const int num_bars = 10;
	//This makes the range periodical!
	//Since x is in [0, 1], then 2x is in [0, 2]
	//But since we are calculating the fractional part only
	//It goes twice from [0, 1]. Then instead of 2 we make it going 10 times
	//float new_x = fract(num_bars * fPosition.x);
	//fragcolor = mix(red, blue, step(0.5, new_x));
	
	//part4
	//const int num_bars = 10;
	//const float amplitude = 0.5;
	//const float frequency = 40.0;
	//float translation_x = amplitude * sin(frequency * fPosition.y);
	//float new_x = fract(num_bars * fPosition.x + translation_x);
	//fragcolor = mix(red, blue, step(0.5, new_x));
	
	//part5
	//const vec2 translation = vec2(-0.5, -0.5);
	//vec2 new_coordinates = fPosition + translation;
	//float r = length(new_coordinates);
	//const float radio = 0.25;
	//fragcolor = mix(red, blue, step(radio, r));
	
	//part5.5
	// const vec2 translation = vec2(-0.5, -0.5);
	// vec2 new_coordinates = fPosition + translation;
	// float r = length(new_coordinates);
	// const float outter_radio = 0.4;
	// const float inner_radio = 0.35;
	// float mid_point = 0.5 * (outter_radio + inner_radio);
    // vec4 color_inside = mix(red, blue, step(inner_radio, r));
	// vec4 color_circle = mix(blue, red, step(outter_radio, r));
	// fragcolor = mix(color_inside, color_circle, step(mid_point, r));
	
	//part6
	//const ivec2 rep_times = ivec2(10, 10);
	//vec2 new_coordinates = fract(rep_times * fPosition);
	//const vec2 translation = vec2(-0.5, -0.5);
	//float r = length(new_coordinates + translation);
	//const float relative_radio = 0.35;
	//fragcolor = mix(red, blue, step(relative_radio, r));
	
	//part7
	const ivec2 rep_times = ivec2(10, 10);
	vec2 new_coordinates = fract(rep_times * fPosition);
	const vec2 translation = vec2(-0.5, -0.5);
	float r = length(new_coordinates + translation);
	const float relative_outter_radio = 0.4;
	const float realtive_inner_radio = 0.3;
	float mid_point = 0.5 * (relative_outter_radio + realtive_inner_radio);
	vec4 color_inside = mix(red, blue, step(realtive_inner_radio, r));
	vec4 color_circle = mix(blue, red, step(relative_outter_radio, r));
	fragcolor = mix(color_inside, color_circle, step(mid_point, r));
}






