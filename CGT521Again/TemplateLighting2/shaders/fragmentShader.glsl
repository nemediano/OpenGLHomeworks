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

vec3 phong_shading(Material m);
vec3 cook_torrance(Material m);

subroutine uniform shadingModel selectedModel;

void main(void) {
	Material m;
	//Fill the material either with the texture or from the current material
	m.Ka = mix(current_material.Ka, vec3(texture2D(texture_map, fTextCoord)), texture_option);
	m.Kd = mix(current_material.Kd, m.Ka, texture_option);
	m.Ks = mix(current_material.Ks, vec3(1.0f), texture_option);
	m.shine = mix(current_material.shine, 32.0f, texture_option);
	
	fragshading = vec4(selectedModel(m), 1.0);
}

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
	vec3 diffuse_term = mat.Kd * light.Ld * max(0.0, dot(n, l));
	
	//Create the specular term using Cook-Torrance
	//Parameters
	float m = 0.03f;
	float eta = 6.16f;
	
	float n_dot_h = dot(n, h);
	float n_dot_v = dot(n, v);
	
	float f_lambda = ((1.0f - eta) / (1.0f + eta)) * ((1.0f - eta) / (1.0f + eta));
	float F = f_lambda + (1.0f - f_lambda) * (1.0f - n_dot_v) * (1.0f - n_dot_v) * (1.0f - n_dot_v) * (1.0f - n_dot_v) * (1.0f - n_dot_v);
	
	float D = exp(-(1.0f - n_dot_h * n_dot_h) / (n_dot_h * n_dot_h)) / (4.0f * m * m * n_dot_h * n_dot_h * n_dot_h * n_dot_h);
	
	float G = min(1.0f, min((2.0f * n_dot_h * n_dot_v)/dot(v, h), (2.0f * n_dot_h * dot(n, l)) / dot(v, h)));
	
	vec3 specular_term = mat.Ks * light.Ls * ((F * D * G)/(3.1416f * dot(n, l) * n_dot_v));
	
	return ambient_term + diffuse_term + specular_term;
}
