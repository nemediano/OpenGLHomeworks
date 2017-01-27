#version 140
in vec3 fNormal;
in vec2 fTextCoord;
flat in int finstanceID;

uniform float time;

out vec4 fragcolor;

const int NCLRS = 9;

vec3 light_dir = vec3(1.0);

vec3 colors[NCLRS] = vec3[](
	vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0),
	vec3(0.5, 0.5, 0.5), vec3(1.0, 1.0, 1.0), vec3(0.8, 0.6, 0.6)
							);

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
	const float ambient_contribution = 0.2;
	
	vec3 color = diffuse * colors[finstanceID % NCLRS] + ambient_contribution * colors[finstanceID % NCLRS] + specular * vec3(1.0);
	
	fragcolor = vec4(color, 1.0);
}
