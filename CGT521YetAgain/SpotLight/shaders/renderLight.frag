#version 430
in vec3 fNormal;

struct Light {
	vec3 La;
	vec3 Ls;
	vec3 Ld;
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
uniform vec3 cameraPosition;

out vec4 fragcolor;

vec3 applyGamma(vec3 color);

void main(void) {
	//Artificial directional light form the camera position
	vec3 l = normalize(-cameraPosition);
	vec3 n = normalize(fNormal);
	
	vec3 Ka = mat.Ka;
	vec3 Kd = mat.Kd;
	
	vec3 La = light.La;
	vec3 Ld = light.Ld;

	vec3 ambient_color = Ka * La;
	vec3 diffuse_color = Kd * Ld * max(0.0, dot(n, l));

	fragcolor = vec4(applyGamma(ambient_color + diffuse_color), 1.0);
}

vec3 applyGamma(vec3 color) {
	return pow(color, vec3(1.0 / gamma));
}
