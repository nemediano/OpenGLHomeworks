#version 430
in vec2 fTextCoord;

//Textures form the FBO
uniform sampler2D colorTexture;

layout (location = 0) out vec4 fragcolor;

void main(void) {
	ivec2 fPos = ivec2(gl_FragCoord.xy);
	vec3 dX = texelFetch(colorTexture, ivec2(fPos.x - 1, fPos.y), 0).rgb - texelFetch(colorTexture, ivec2(fPos.x + 1, fPos.y), 0).rgb;
	vec3 dY = texelFetch(colorTexture, ivec2(fPos.x, fPos.y - 1), 0).rgb - texelFetch(colorTexture, ivec2(fPos.x, fPos.y + 1), 0).rgb;
	vec3 color = dX * dX + dY * dY;
	fragcolor = vec4(min(color, vec3(1.0)), 1.0);
}
