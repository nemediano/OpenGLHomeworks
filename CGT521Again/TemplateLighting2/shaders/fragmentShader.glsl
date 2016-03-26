#version 430

//Inputs from vertex shader
in vec3 fNormal;
//Position of the fragment in view space
in vec3 fPosition;
//Texture coordinates of this fragment
in vec2 fTextCoord;

layout (location = 0) out vec4 fragshading;

struct LightSource {
	vec3 La;
	vec3 Ld;
	vec3 Ls;
	vec3 position;
};

struct Material {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float shine;
};

uniform Material current_material;
uniform LightSource light;

uniform int texture_option;
uniform float time;

//Texture map
uniform sampler2D texture_map;

subroutine vec3 shadingModel(Material m);

//vec3 phong_shading(Material m);
//vec3 cook_torrance(Material m);
//Helpers for Cook Torrance
float geometric_attenuation(vec3 n, vec3 h, vec3 v, vec3 l);
float roughness_term(vec3 n, vec3 h, float m);
float fresnel_term(vec3 h, vec3 v, float eta);
float fresnel_term_fast(vec3 n, vec3 v, float eta);
float fresnel_term_2(vec3 n, vec3 v, float eta);

subroutine uniform shadingModel selectedModel;

subroutine (shadingModel) vec3 phong_shading(Material mat) {
	
	vec3 n = fNormal;
	//Light position must be in Viewspace (Same as fPosition)
	vec3 l = normalize(light.position - fPosition);
	//Since we are in view space the eye is at the origin so v = eye - p = -p
	vec3 v = normalize(-fPosition);
	//Since in GLSL the first argument of the reflect function 
	//is "incident" vector. I'm going to use the negative of the
	//vector that usually use on the books
	vec3 r = normalize(reflect(-l, fNormal));

	vec3 ambient_term = mat.Ka * light.La;
	vec3 diffuse_term = mat.Kd * light.Ld * max(0.0, dot(n, l));
	vec3 specular_term = mat.Ks * light.Ls * pow(max(0.0, dot(r, v)), mat.shine);
	
	return ambient_term + diffuse_term + specular_term;
}

subroutine (shadingModel) vec3 cook_torrance(Material mat) {
	vec3 n = fNormal;
	//Light position must be in Viewspace (Same as fPosition)
	vec3 l = normalize(light.position - fPosition);
	//Since we are in view space the eye is at the origin so v = eye - p = -p
	vec3 v = normalize(-fPosition);
	//Since in GLSL the first argument of the reflect function 
	//is "incident" vector. I'm going to use the negative of the
	//vector that usually use on the books
	vec3 r = normalize(reflect(-l, fNormal));
	//Half vector used in this model
	vec3 h = normalize(l + v);

	vec3 ambient_term = mat.Ka * light.La;
	vec3 diffuse_term = mat.Kd * light.Ld * max(0.0f, dot(n, l));
	
	//Create the specular term using Cook-Torrance
	//Parameters
	float m = 0.9f;
	float eta = 0.7f;
	//float F = fresnel_term(h, v, eta);
	//float F = fresnel_term_fast(n, v, eta);
	float F = fresnel_term_2(n, v, eta);
	//float F = 1.0f;
	float D = roughness_term(n, h, m);
	//float D = 1.0f;
	float G = geometric_attenuation(n, h, v, l);
	//float G = 1.0f;
	
	vec3 specular_term = mat.Ks * light.Ls * max(0.0f,(F * D * G)/(4.0f * dot(n, l) * dot(n, v)));
	//vec3 specular_term = mat.Ks * light.Ls * max(0.0f, F);
	
	return ambient_term + diffuse_term + specular_term;
	//return specular_term;
}


void main(void) {
	Material m;
	//Fill the material either with the texture or from the current material
	m.Ka = mix(current_material.Ka, vec3(texture2D(texture_map, fTextCoord)), texture_option);
	m.Kd = mix(current_material.Kd, m.Ka, texture_option);
	m.Ks = mix(current_material.Ks, vec3(1.0f), texture_option);
	m.shine = mix(current_material.shine, 32.0f, texture_option);
	
	fragshading = vec4(selectedModel(m), 1.0);
}


float geometric_attenuation(vec3 n, vec3 h, vec3 v, vec3 l) {
	
	float n_dot_h = dot(n, h);
	float v_dot_h = dot(v, h);
	
	float masking = 2.0f * n_dot_h * dot(n, v) / v_dot_h;
	float shadowing = 2.0f * n_dot_h * dot(n, l) / v_dot_h;
	
	return min(1.0f, min(masking, shadowing));
}

float roughness_term(vec3 n, vec3 h, float m) {
	float n_dot_h_sq = dot(n, h) * dot(n, h);
	float tan_sq = (1.0f - n_dot_h_sq) / (n_dot_h_sq);
	float m_sq = m * m;
	
	return exp(-1.0f * tan_sq / (m_sq))/(3.1416f * m_sq * n_dot_h_sq * n_dot_h_sq);
}

float fresnel_term_fast(vec3 n, vec3 v, float eta) {
	float one_minus_n_dot_v_5th = pow(1.0f - dot(n, v), 5.0);
	float f_lambda = ((1.0f - eta) / (1.0f + eta)) * ((1.0f - eta) / (1.0f + eta));
	
	return f_lambda + (1.0f - f_lambda) * one_minus_n_dot_v_5th;
}

float fresnel_term(vec3 h, vec3 v, float eta) {
	float c = dot(v, h);
	float g = sqrt(eta * eta + c * c - 1.0);
	
	float g_plus_c = g + c;
	float g_minus_c = g - c;
	
	float left_factor  =  (g_minus_c * g_minus_c) / (2.0 * g_plus_c * g_plus_c);
	float right_factor = (1.0 + pow(c * g_plus_c - 1.0, 2.0) / pow(c * g_minus_c - 1.0, 2.0));
	
	return left_factor * right_factor;
}

float fresnel_term_2(vec3 n, vec3 v, float eta) {
	return pow(1.0 + dot(n, v), eta);
}