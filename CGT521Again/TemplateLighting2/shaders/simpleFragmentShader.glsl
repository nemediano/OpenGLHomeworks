#version 430

uniform sampler2D color_map;
uniform sampler2D color_shade_map;
uniform sampler2D normal_map;

out vec4 fragcolor;

subroutine vec4 filterType();

vec4 no_filter();
vec4 average_3x3();
vec4 average_9x9();
vec4 edge_detection();
vec4 sobel_filter();
vec4 sobel_threshold();
vec4 sobel_threshold_normals();
      
ivec2 safe_position(ivec2 position);

subroutine uniform filterType selectedFilter;

const vec4 black = vec4(0.0f, 0.0f, 0.0f, 1.0f);
const vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  
void main(void) {
	
	fragcolor = selectedFilter();
	
}

//Calculate safe coordinates to evaluate texture 
//and avoid branching
ivec2 safe_position(ivec2 position) {
	//All the textures have the same size!
	ivec2 size	= textureSize(color_shade_map, 0);
	return ivec2(mod(position + size, size));
}

subroutine (filterType) vec4 no_filter() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	return texelFetch(color_shade_map, fPosition, 0);
}

subroutine (filterType) vec4 average_3x3() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	//Some helpers for doing the average filtering
	ivec2 i = ivec2(1, 0);
	ivec2 j = ivec2(0, 1);
	vec4 color = vec4(0.0f);
	//Average filtering
	color += texelFetch(color_shade_map, safe_position(fPosition - i - j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition     - j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition + i - j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition - i    ), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition        ), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition + i    ), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition - i + j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition     + j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition + i + j), 0);
	
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
	color += texelFetch(color_shade_map, safe_position(fPosition - 2 * i - 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition -     i - 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition         - 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition +     i - 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition + 2 * i - 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition - 2 * i -     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition -     i -     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition         -     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition +     i -     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition + 2 * i -     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition - 2 * i        ), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition -     i        ), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition                ), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition +     i        ), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition + 2 * i        ), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition - 2 * i +     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition -     i +     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition         +     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition +     i +     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition + 2 * i +     j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition - 2 * i + 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition -     i + 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition         + 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition +     i + 2 * j), 0);
	color += texelFetch(color_shade_map, safe_position(fPosition + 2 * i + 2 * j), 0);
	
	return (1.0f / 25.0f) * color;
}

subroutine (filterType) vec4 edge_detection() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	//Some helpers for doing the average filtering
	ivec2 i = ivec2(1, 0);
	ivec2 j = ivec2(0, 1);
	
	vec4 left  = texelFetch(color_map, safe_position(fPosition - i), 0);
	vec4 right = texelFetch(color_map, safe_position(fPosition + i), 0);
	vec4 above = texelFetch(color_map, safe_position(fPosition + j), 0);
	vec4 below = texelFetch(color_map, safe_position(fPosition - j), 0);
	
	return (left - right) * (left - right) + (above - below) * (above - below);
}

subroutine (filterType) vec4 sobel_filter() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	//Some helpers for doing the average filtering
	ivec2 i = ivec2(1, 0);
	ivec2 j = ivec2(0, 1);
	
	vec4 dx = vec4(0.0f);
	//Sobel filtering in dx
	dx += texelFetch(color_map, safe_position(fPosition - i - j), 0);
	dx -= texelFetch(color_map, safe_position(fPosition + i - j), 0);
	dx += 2.0f * texelFetch(color_map, safe_position(fPosition - i    ), 0);
	dx -= 2.0f * texelFetch(color_map, safe_position(fPosition + i    ), 0);
	dx += texelFetch(color_map, safe_position(fPosition - i + j), 0);
	dx -= texelFetch(color_map, safe_position(fPosition + i + j), 0);
	
	vec4 dy = vec4(0.0f);
	dy += texelFetch(color_map, safe_position(fPosition - i - j), 0);
	dy += 2.0f * texelFetch(color_map, safe_position(fPosition     - j), 0);
	dy += texelFetch(color_map, safe_position(fPosition + i - j), 0);
	dy -= texelFetch(color_map, safe_position(fPosition - i + j), 0);
	dy -= 2.0f * texelFetch(color_map, safe_position(fPosition     + j), 0);
	dy -= texelFetch(color_map, safe_position(fPosition + i + j), 0);
	
	return dx * dx + dy * dy;
}

subroutine (filterType) vec4 sobel_threshold() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	//Some helpers for doing the average filtering
	ivec2 i = ivec2(1, 0);
	ivec2 j = ivec2(0, 1);
	
	vec4 dx = vec4(0.0f);
	//Sobel filtering in dx
	dx += texelFetch(color_map, safe_position(fPosition - i - j), 0);
	dx -= texelFetch(color_map, safe_position(fPosition + i - j), 0);
	dx += 2.0f * texelFetch(color_map, safe_position(fPosition - i    ), 0);
	dx -= 2.0f * texelFetch(color_map, safe_position(fPosition + i    ), 0);
	dx += texelFetch(color_map, safe_position(fPosition - i + j), 0);
	dx -= texelFetch(color_map, safe_position(fPosition + i + j), 0);
	
	vec4 dy = vec4(0.0f);
	dy += texelFetch(color_map, safe_position(fPosition - i - j), 0);
	dy += 2.0f * texelFetch(color_map, safe_position(fPosition     - j), 0);
	dy += texelFetch(color_map, safe_position(fPosition + i - j), 0);
	dy -= texelFetch(color_map, safe_position(fPosition - i + j), 0);
	dy -= 2.0f * texelFetch(color_map, safe_position(fPosition     + j), 0);
	dy -= texelFetch(color_map, safe_position(fPosition + i + j), 0);
	
	float edge = length(dx * dx + dy * dy);
	
	return mix(white, black, edge);
}

subroutine (filterType) vec4 sobel_threshold_normals() {
	ivec2 fPosition = ivec2(gl_FragCoord.xy);
	//Some helpers for doing the average filtering
	ivec2 i = ivec2(1, 0);
	ivec2 j = ivec2(0, 1);
	
	vec4 dx = vec4(0.0f);
	//Sobel filtering in dx
	dx += texelFetch(normal_map, safe_position(fPosition - i - j), 0);
	dx -= texelFetch(normal_map, safe_position(fPosition + i - j), 0);
	dx += 2.0f * texelFetch(normal_map, safe_position(fPosition - i    ), 0);
	dx -= 2.0f * texelFetch(normal_map, safe_position(fPosition + i    ), 0);
	dx += texelFetch(normal_map, safe_position(fPosition - i + j), 0);
	dx -= texelFetch(normal_map, safe_position(fPosition + i + j), 0);
	
	vec4 dy = vec4(0.0f);
	dy += texelFetch(normal_map, safe_position(fPosition - i - j), 0);
	dy += 2.0f * texelFetch(normal_map, safe_position(fPosition     - j), 0);
	dy += texelFetch(normal_map, safe_position(fPosition + i - j), 0);
	dy -= texelFetch(normal_map, safe_position(fPosition - i + j), 0);
	dy -= 2.0f * texelFetch(normal_map, safe_position(fPosition     + j), 0);
	dy -= texelFetch(normal_map, safe_position(fPosition + i + j), 0);
	
	//float edge = 1.0f - length(dx * dx + dy * dy);
	float edge = length(dx * dx + dy * dy);
	
	return mix(white, black, edge);
	//return vec4(edge * vec3(1.0f), 1.0f);
}



