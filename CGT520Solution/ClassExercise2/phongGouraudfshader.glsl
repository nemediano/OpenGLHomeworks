#version 150                                                                                                               

in vec3 vcolor;
in vec2 vtext_coordinates;
out vec4 fragcolor;                         //output fragment color

uniform int texture_option; //Use texture or color
uniform sampler2D texture_map; //texture map

void main(void) {    
	vec4 text_color = texture(texture_map, vtext_coordinates);
	vec4 text_plus_color = mix(text_color, vec4(vcolor, 1.0), 0.20);
	fragcolor = mix(text_plus_color, vec4(vcolor, 1.0), texture_option);  
}