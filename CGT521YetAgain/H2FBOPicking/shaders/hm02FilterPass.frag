#version 430
in vec2 fTextCoord;

//Textures form the FBO
uniform sampler2D colorTexture;

layout (location = 0) out vec4 fragcolor;

void main(void) {
	ivec2 fPos = ivec2(gl_FragCoord.xy);
	vec3 color = texelFetch(colorTexture, fPos, 0).rgb;
	fragcolor = vec4(min(color, vec3(1.0)), 1.0);
}
