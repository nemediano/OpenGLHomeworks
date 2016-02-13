#version 330

uniform sampler2D texture;
//uniform int blur;

out vec4 fragcolor;           
      
void main(void) {   
	
	fragcolor = texelFetch(texture, ivec2(gl_FragCoord.xy), 0);
	
}