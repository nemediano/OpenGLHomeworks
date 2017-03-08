#version 430
in vec3 fNormal;
in vec2 fTextCoord;
in flat vec3 fColor;
//Instance id of this mesh
in flat int fInstanceId;

uniform float time;
uniform int selected;
layout (binding = 0) uniform sampler2D textureMap;

layout (location = 0) out vec3 fragcolor;
layout (location = 1) out vec3 instanceId;

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
	vec3 difusseColor  = texture(textureMap, fTextCoord).rgb;
	vec3 color = diffuse * difusseColor + ambient * fColor + specular * vec3(0.85);
	if (selected == fInstanceId + 1) {
		color = 0.8 * fColor + specular * vec3(0.85);
	}
	 
	fragcolor = min(color, vec3(1.0));
	
	instanceId = float(fInstanceId + 1) / 255.0f * vec3(1.0f, 0.0f, 0.0f);
}
