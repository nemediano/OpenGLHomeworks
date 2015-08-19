#version 150                                                                                                               

uniform sampler2D texture;
uniform vec4 uColor;
uniform float time;

in vec2 vtexcoord;
in vec3 vnormal;

out vec4 fragcolor;                         //output fragment color

void main(void)
{        

	vec2 f = abs(fract(50.0*vtexcoord.xy)-vec2(0.5, 0.5));
	float d = dot(f, f);
	//float d = min(f.x, f.y);
	//float a = smoothstep(0.0, 0.4, d);
	float a = smoothstep(0.1, 0.15, d) - smoothstep(0.15, 0.2, d);
	vec3 l = vec3(0.707, 0.0, 0.707);
	vec4 diffuse_color = mix(vec4(0.2, 0.2, 0.2, 1.0), uColor, a);

	fragcolor = diffuse_color*max(0.0, dot(vnormal, l)); 
         
}