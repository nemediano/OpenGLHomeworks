#version 400                                                                                                               

uniform sampler2D colortex;
uniform float time;

in vec4 vcolor;
out vec4 fragcolor;                         //output fragment color

void main(void)
{        
	fragcolor = vcolor * texture(colortex, gl_PointCoord.st);
}