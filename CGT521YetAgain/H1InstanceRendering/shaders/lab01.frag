#version 140
in vec3 fNormal;
in vec2 fTextCoord;
flat in vec3 fColor;

uniform float time;

out vec4 fragcolor;

vec3 light_dir = vec3(1.0);

void main(void) {
	//Set up light
	const vec3 eye = vec3(0.0, 0.0, 1.0);
	vec3 light_dir = vec3(0.0, 1.0, 1.0);
	light_dir += vec3(sin(time), 0.0, 0.0);
	
	vec3 n = normalize(fNormal);
	vec3 l = normalize(light_dir);
	vec3 v = normalize(eye);
	vec3 r = reflect(-l, n);
	float diffuse = max(0.0, dot(n, l));
	const float alpha = 16.0;
	float specular = pow(max(0.0, dot(r, v)), alpha);
	const float ambient = 0.2;
	
	vec3 color = (diffuse + ambient) * fColor + specular * vec3(0.85);
	
	fragcolor = vec4(color, 1.0);
}
