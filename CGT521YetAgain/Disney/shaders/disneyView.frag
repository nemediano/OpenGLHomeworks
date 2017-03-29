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
	float m;
	float eta;
};

uniform Light light;
uniform Material mat;
uniform float gamma = 1.0;
uniform int option = 0;


out vec4 fragcolor;

float geometric_attenuation(vec3 n, vec3 h, vec3 v, vec3 l);
float roughness_term(vec3 n, vec3 h, float m);
float fresnel_term(vec3 h, vec3 v, float eta);

const float EPSILON = 0.0000001;

void main(void) {
	vec3 color = vec3(0.0);
	vec3 l = normalize(light.position - fPosition);
	vec3 n = normalize(fNormal);
	vec3 v = normalize(-fPosition);
	vec3 r = normalize(reflect(-l, n));
	vec3 h = normalize(l + v);
	
	vec3 Ka = mat.Ka;
	vec3 Kd = mat.Kd;
	vec3 Ks = mat.Ks;
	float eta = mat.eta;
	float m = mat.m;
	
	vec3 La = light.La;
	vec3 Ld = light.Ld;
	vec3 Ls = light.Ls;
	
	float F = fresnel_term(h, v, eta);
	float D = roughness_term(n, h, m);
	float G = geometric_attenuation(n, h, v, l);
	
	vec3 ambient_color = Ka * La;
	vec3 diffuse_color = Kd * Ld * max(0.0, dot(n, l));
	vec3 speculr_color;
	
	float n_dot_l = dot(n, l);
	float n_dot_v = max(0.0, dot(n, v));
	
	if (option == 1) {
		speculr_color = max(0.0, F / (4.0 * n_dot_l * n_dot_v)) * vec3(1.0);
	} else if (option == 2) {
		speculr_color = max(0.0, D / (4.0 * n_dot_l * n_dot_v)) * vec3(1.0);
	} else if (option == 3) {
		speculr_color = max(0.0, G / (4.0 * n_dot_l * n_dot_v)) * vec3(1.0);
	} else {
		speculr_color = mat.Ks * light.Ls * max(0.0, (F * D * G) / (4.0 * n_dot_l * n_dot_v));
	}
	
	color = ambient_color + diffuse_color + speculr_color;
	
	fragcolor = vec4(pow(color, vec3(1.0 / gamma)), 1.0);
}

float geometric_attenuation(vec3 n, vec3 h, vec3 v, vec3 l) {
	
	float n_dot_h = max(0.0, dot(n, h));
	float v_dot_h = max(EPSILON, dot(v, h));
	float n_dot_v = max(0.0, dot(n, v));
	float n_dot_l = dot(n, l);
	
	float masking = 2.0f * n_dot_h * n_dot_v / v_dot_h;
	float shadowing = 2.0f * n_dot_h * n_dot_l / v_dot_h;
	
	return min(1.0f, min(masking, shadowing));
}

float roughness_term(vec3 n, vec3 h, float m) {
	float n_dot_h_sq = dot(n, h) * dot(n, h);
	float tan_sq = (1.0f - n_dot_h_sq) / (n_dot_h_sq);
	float m_sq = m * m;
	
	return exp(-1.0f * tan_sq / (m_sq)) / (3.1416f * m_sq * n_dot_h_sq * n_dot_h_sq);
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