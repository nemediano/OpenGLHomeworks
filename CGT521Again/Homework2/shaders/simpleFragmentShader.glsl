#version 430

uniform sampler2D texture_map;

out vec4 fragcolor;

subroutine vec4 filterType();

vec4 no_filter();
vec4 average_3x3();
vec4 average_9x9();
vec4 edge_detection();          
ivec2 safe_position(ivec2 position);

subroutine uniform filterType selectedFilter;
      
void main(void) {
	
	fragcolor = selectedFilter();
	
}

//Calculate safe coordinates to evaluate texture 
//and avoid branching
ivec2 safe_position(ivec2 position) {
	ivec2 size	= textureSize(texture_map, 0);
	return ivec2(mod(position + size, size));
}

subroutine (filterType) vec4 no_filter() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	return texelFetch(texture_map, fPosition, 0);
}

subroutine (filterType) vec4 average_3x3() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	//Some helpers for doing the average filtering
	ivec2 i = ivec2(1, 0);
	ivec2 j = ivec2(0, 1);
	vec4 color = vec4(0.0f);
	//Average filtering
	color += texelFetch(texture_map, safe_position(fPosition - i - j), 0);
	color += texelFetch(texture_map, safe_position(fPosition     - j), 0);
	color += texelFetch(texture_map, safe_position(fPosition + i - j), 0);
	color += texelFetch(texture_map, safe_position(fPosition - i    ), 0);
	color += texelFetch(texture_map, safe_position(fPosition        ), 0);
	color += texelFetch(texture_map, safe_position(fPosition + i    ), 0);
	color += texelFetch(texture_map, safe_position(fPosition - i + j), 0);
	color += texelFetch(texture_map, safe_position(fPosition     + j), 0);
	color += texelFetch(texture_map, safe_position(fPosition + i + j), 0);
	
	return (1.0f / 9.0f) * color;
}

subroutine (filterType) vec4 average_9x9() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	//Some helpers for doing the average filtering
	ivec2 i = ivec2(1, 0);
	ivec2 j = ivec2(0, 1);
	vec4 color = vec4(0.0f);
	
	//Average filtering using a 5 x 5 neighbourhood 
	//Avoding the nested for loops at all cost :P
	color += texelFetch(texture_map, safe_position(fPosition - 2 * i - 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition -     i - 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition         - 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition +     i - 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition + 2 * i - 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition - 2 * i -     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition -     i -     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition         -     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition +     i -     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition + 2 * i -     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition - 2 * i        ), 0);
	color += texelFetch(texture_map, safe_position(fPosition -     i        ), 0);
	color += texelFetch(texture_map, safe_position(fPosition                ), 0);
	color += texelFetch(texture_map, safe_position(fPosition +     i        ), 0);
	color += texelFetch(texture_map, safe_position(fPosition + 2 * i        ), 0);
	color += texelFetch(texture_map, safe_position(fPosition - 2 * i +     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition -     i +     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition         +     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition +     i +     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition + 2 * i +     j), 0);
	color += texelFetch(texture_map, safe_position(fPosition - 2 * i + 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition -     i + 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition         + 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition +     i + 2 * j), 0);
	color += texelFetch(texture_map, safe_position(fPosition + 2 * i + 2 * j), 0);
	
	return (1.0f / 25.0f) * color;
}

subroutine (filterType) vec4 edge_detection() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	//Some helpers for doing the average filtering
	ivec2 i = ivec2(1, 0);
	ivec2 j = ivec2(0, 1);
	
	vec4 left  = texelFetch(texture_map, safe_position(fPosition - i), 0);
	vec4 right = texelFetch(texture_map, safe_position(fPosition + i), 0);
	vec4 above = texelFetch(texture_map, safe_position(fPosition + j), 0);
	vec4 below = texelFetch(texture_map, safe_position(fPosition - j), 0);
	
	return (left - right) * (left - right) + (above - below) * (above - below);
}

