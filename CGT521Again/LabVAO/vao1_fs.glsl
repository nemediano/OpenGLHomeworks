#version 400
out vec4 fragcolor;      
in vec2 scaled_position;
void main(void)
{   
	// part1
	//fragcolor = vec4(scaled_position.x, scaled_position.y, 0.0, 1.0);
	
	// part2
	//vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
	//vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
	//fragcolor = mix(red, blue, step(0.5, scaled_position.x));
	
	// part3
	//vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
	//vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
	//fragcolor = mix(red, blue, step(0.5, fract(10.0 * scaled_position.x)));
	
	// part3
	//vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
	//vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
	//float new_x = fract(10.0 * scaled_position.x + 0.25 * sin(60.0 * scaled_position.y));
	//fragcolor = mix(red, blue, step(0.5, new_x));
	
	
	// part4
	vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
	vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
    vec2 new_x = scaled_position - vec2(0.5);
	//Convert to polar coordiantes
	float radius = length(new_x);
	fragcolor = mix(red, blue, step(0.25, radius));
}




















