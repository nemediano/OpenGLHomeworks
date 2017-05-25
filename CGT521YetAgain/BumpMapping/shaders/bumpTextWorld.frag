#version 430
in vec2 fTextCoord;
in vec3 fPosition;

struct Light {
	vec3 position;
	vec3 color;
	float intensity;
	float ratio;
};

struct Material {
	float metalicity;
	float roughness;
};

uniform Light light;
uniform Material mat;
uniform float gamma = 1.0;
uniform vec3 cameraPosition;
uniform mat4 NormalMat;
uniform sampler2D normalsMap;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

out vec4 fragcolor;

float geometric_term(float roughness, vec3 L_or_V, vec3 n);
float distribution_term(float NdH, float alpha2);
vec3 fresnel_term(float VdH, vec3 F0);
vec3 gammaCorrection(vec3 color);
vec3 toneMapping(vec3 color);

const float EPSILON = 1e-7;
const float PI = 3.14159;

void main(void) {
	vec3 color = vec3(0.0);
	vec3 l = normalize(light.position - fPosition);
	vec3 n = normalize((NormalMat * vec4(texture(normalsMap, fTextCoord).xyz, 0.0)).xyz);
	vec3 v = normalize(cameraPosition - fPosition);
	vec3 h = normalize(l + v);
	
	vec3 baseColor = texture(diffuseMap, fTextCoord).rgb;;
	vec3 F0 = texture(specularMap, fTextCoord).rgb;
	float roughness = mat.roughness;
	float metalicity = mat.metalicity;
	
	vec3 lightColor = light.color;
	float intensity = light.intensity;
	
	float n_dot_l = max(0.0, dot(n, l));
	float n_dot_v = max(0.0, dot(n, v));
	float v_dot_h = max(0.0, dot(v, h));
	float n_dot_h = max(0.0, dot(n, h));
	//No light radious (Directional light?)
	float lighRadius = light.ratio;
	float alpha2 = roughness * roughness + lighRadius;
	
	vec3  F = fresnel_term(v_dot_h, F0);
	float D = distribution_term(n_dot_h, alpha2);
	float Gl = geometric_term(roughness, l, n);
	float Gv = geometric_term(roughness, v, n);
	
	// Ambient lighting, to fake an image based map
	vec3 ambient_color = 0.25 * baseColor;
	// metals don't have a diffuse contribution, so turn off the diffuse color
	// when the material is metallic
	vec3 diffuse_color = ((1.0 - metalicity) / PI) * baseColor;
	//use reflectance to calculate energy conservation
	//diffuse_color *= vec3(1.0) - F0;
	vec3 speculr_color = ((Gl * Gv * D) / (max(EPSILON, 4.0 * n_dot_l * n_dot_v))) * F;
	
	color = ambient_color + (lightColor * intensity) * (n_dot_l * (diffuse_color + speculr_color));
	
	fragcolor = vec4(gammaCorrection(color), 1.0);
}

float geometric_term(float roughness, vec3 L_or_V, vec3 n) {
	// remapped roughness, to be used for the geometry term calculations,
	// per Disney [16], Unreal [3]. N.B. don't do this in IBL
	float roughness_remapped = 0.5 + roughness / 2.0;

	float NdV = max(0.0, dot(n, L_or_V));

	float k = pow(roughness_remapped + 1.0, 2.0) / 8.0;
	return NdV / ((NdV) * (1.0 - k) + k);
}
//Also known as roughness term
float distribution_term(float NdH, float alpha2) {
	float D_denominator = ((NdH * NdH)*(alpha2 - 1.0) + 1.0);
	return alpha2 / (max(PI * D_denominator * D_denominator, EPSILON));
}

// Schlick approximation
vec3 fresnel_term(float VdH, vec3 F0) {	
	return F0 + (vec3(1.0) - F0) * pow(2.0, (-5.55473 * VdH - 6.98316) * VdH);
}

vec3 gammaCorrection(vec3 color) {
	return pow(color, vec3(1.0 / gamma));
}

vec3 toneMapping(vec3 color) {
	return color / (color + vec3(1.0));
}