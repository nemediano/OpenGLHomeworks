#version 150                                                                                                               

uniform sampler2D texture;
uniform vec4 uColor;

in vec2 vtexcoord;
out vec4 fragcolor;                         //output fragment color

void main(void)
{        
	fragcolor = uColor;          
}