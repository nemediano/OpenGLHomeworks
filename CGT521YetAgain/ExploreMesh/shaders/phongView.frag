#version 430
in vec3 fNormal;
in vec2 fTextCoord;
in vec3 fPosition;

struct Light {
	vec3 La;
	vec3 Ls;
	vec3 Ld;
	vec3 position;
};

struct Material {
	vec3 Ka;
	vec3 Ks;
	vec3 Kd;
	float alpha;
};

uniform Light light;
uniform Material mat;
uniform float gamma = 1.0;
layout(binding = 0) uniform sampler2D diffuseMap;
uniform float mixValue = 1.0;

out vec4 fragcolor;

void main(void) {
	vec3 color = vec3(0.0);
	//Directional light (in view space)
	vec3 l = vec3(0.0, 0.0, 1.0);
	vec3 n = normalize(fNormal);
	vec3 v = normalize(-fPosition);
	vec3 r = normalize(reflect(-l, n));
	
	vec3 Ka = mat.Ka;
	vec3 Kd = mat.Kd; 
	vec3 Ks = mat.Ks;
	float alpha = mat.alpha;
	
	vec3 La = light.La;
	vec3 Ld = light.Ld;
	vec3 Ls = light.Ls;
	
	vec3 ambient_color = Ka * La;
	vec3 diffuse_color = Kd * Ld * max(0.0, dot(n, l));
	vec3 speculr_color = Ks * Ls * pow(max(0.0, dot(r, v)), alpha);
	
	color = mix(ambient_color + diffuse_color + speculr_color, texture(diffuseMap, fTextCoord).rgb, mixValue);
		
	fragcolor = vec4(pow(color, vec3(1.0 / gamma)), 1.0);
}
