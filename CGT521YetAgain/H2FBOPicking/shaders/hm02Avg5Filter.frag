#version 430
in vec2 fTextCoord;

//Textures form the FBO
uniform sampler2D colorTexture;

layout (location = 0) out vec4 fragcolor;

void main(void) {
	ivec2 fPos = ivec2(gl_FragCoord.xy);
	
	//Initialize accumulator
	vec3 color = vec3(0);
	color += texelFetch(colorTexture, ivec2(fPos.x - 2, fPos.y - 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 1, fPos.y - 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x    , fPos.y - 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 1, fPos.y - 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 2, fPos.y - 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 2, fPos.y - 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 1, fPos.y - 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x    , fPos.y - 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 1, fPos.y - 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 2, fPos.y - 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 2, fPos.y    ), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 1, fPos.y    ), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x    , fPos.y    ), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 1, fPos.y    ), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 2, fPos.y    ), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 2, fPos.y + 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 1, fPos.y + 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x    , fPos.y + 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 1, fPos.y + 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 2, fPos.y + 1), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 2, fPos.y + 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x - 1, fPos.y + 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x    , fPos.y + 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 1, fPos.y + 2), 0).rgb;
	color += texelFetch(colorTexture, ivec2(fPos.x + 2, fPos.y + 2), 0).rgb;
	color /= 25.0;
	
	fragcolor = vec4(min(color, vec3(1.0)), 1.0);
}
