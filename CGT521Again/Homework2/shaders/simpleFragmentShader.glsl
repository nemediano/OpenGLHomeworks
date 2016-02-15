#version 330

uniform sampler2D texture_map;
//uniform int blur;

out vec4 fragcolor;           
      
void main(void) {   	
	fragcolor = texelFetch(texture_map, ivec2(gl_FragCoord.xy), 0);
}